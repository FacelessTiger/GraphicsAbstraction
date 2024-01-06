#include "D3D12Shader.h"

#include <dxc/dxcapi.h>

#include <GraphicsAbstraction/Core/Log.h>
#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Context.h>

namespace GraphicsAbstraction {

	namespace Utils {

		static LPCWSTR ShaderStageToDXC(ShaderStage stage)
		{
			switch (stage)
			{
				case ShaderStage::Vertex:	return L"vs_6_6";
				case ShaderStage::Pixel:	return L"ps_6_6";
				case ShaderStage::Compute:	return L"cs_6_6";
			}

			GA_CORE_ASSERT(false);
			return nullptr;
		}

	}

	using namespace Microsoft::WRL;

	static bool s_Initialized = false;
	static ComPtr<IDxcUtils> s_Utils;
	static ComPtr<IDxcCompiler3> s_Compiler;
	static ComPtr<IDxcIncludeHandler> s_IncludeHandler;

	static uint32_t s_IDCounter = 1;
	static std::unordered_map<uint32_t, D3D12Shader*> s_ShaderList;

	Ref<Shader> Shader::Create(const std::string& path, ShaderStage stage)
	{
		return CreateRef<D3D12Shader>(path, stage);
	}

	D3D12Shader::D3D12Shader(const std::string& path, ShaderStage stage)
		: Stage(stage), ID(s_IDCounter++)
	{
		if (!s_Initialized)
		{
			D3D12_CHECK(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&s_Utils)));
			D3D12_CHECK(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&s_Compiler)));
			D3D12_CHECK(s_Utils->CreateDefaultIncludeHandler(&s_IncludeHandler));

			s_Initialized = true;
		}

		std::wstring wstring = std::filesystem::path(path).wstring();
		LPCWSTR filePath = wstring.c_str();

		uint32_t codePage = DXC_CP_ACP;
		ComPtr<IDxcBlobEncoding> sourceBlob;

		if (FAILED(s_Utils->LoadFile(filePath, &codePage, &sourceBlob)))
		{
			GA_CORE_ERROR("Could not open file \"{0}\"", path);
			GA_CORE_ASSERT(false);
		}

		// Note: Zi causes debug information
		std::vector<LPCWSTR> arguments = {
			filePath, DXC_ARG_WARNINGS_ARE_ERRORS,
			L"-E", L"main",
			L"-T", Utils::ShaderStageToDXC(stage),
			L"-Zi"
		};

		DxcBuffer buffer = {
			.Ptr = sourceBlob->GetBufferPointer(),
			.Size = sourceBlob->GetBufferSize(),
			.Encoding = DXC_CP_ACP,
		};

		ComPtr<IDxcResult> result;
		D3D12_CHECK(s_Compiler->Compile(&buffer, arguments.data(), arguments.size(), s_IncludeHandler.Get(), IID_PPV_ARGS(&result)));

		HRESULT status;
		result->GetStatus(&status);
		if (FAILED(status) && result)
		{
			ComPtr<IDxcBlobEncoding> errorBlob;
			result->GetErrorBuffer(&errorBlob);

			GA_CORE_ERROR((const char*)errorBlob->GetBufferPointer());
			GA_CORE_ASSERT(false);
		}

		//ComPtr<IDxcBlob> blob;
		result->GetResult(&m_Blob);

		//m_Data.resize(blob->GetBufferSize());
		//memcpy(m_Data.data(), blob->GetBufferPointer(), blob->GetBufferSize());

		Shader = {
			.pShaderBytecode = m_Blob->GetBufferPointer(),
			.BytecodeLength = m_Blob->GetBufferSize()
		};
		s_ShaderList[ID] = this;
	}

	D3D12Shader::~D3D12Shader()
	{
		s_ShaderList.erase(ID);
	}

	D3D12Shader* D3D12Shader::GetShaderByID(uint32_t id)
	{
		auto it = s_ShaderList.find(id);
		if (it == s_ShaderList.end()) return nullptr;

		return it->second;
	}

}