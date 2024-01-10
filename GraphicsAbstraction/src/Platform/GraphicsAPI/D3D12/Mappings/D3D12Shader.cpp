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
		
		std::string file = ReadAndPreProcessFile(path);
		ComPtr<IDxcBlobEncoding> sourceBlob;

		if (s_Utils->CreateBlob(file.c_str(), file.size(), DXC_CP_UTF8, &sourceBlob))
		{
			GA_CORE_ERROR("Could not open file \"{0}\"", path);
			GA_CORE_ASSERT(false);
		}

		std::wstring wstring = std::filesystem::path(path).wstring();
		LPCWSTR filePath = wstring.c_str();

		// Note: Zi causes debug information
		std::vector<LPCWSTR> arguments = {
			filePath, DXC_ARG_WARNINGS_ARE_ERRORS,
			L"-E", L"main",
			L"-T", Utils::ShaderStageToDXC(stage),
			L"-Zi", L"-Od"
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
		result->GetResult(&m_Blob);

//#define OUTPUT_PDB
#ifdef OUTPUT_PDB
		ComPtr<IDxcBlob> debugBlob;
		ComPtr<IDxcBlobUtf16> debugDataPath;
		D3D12_CHECK(result->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&debugBlob), &debugDataPath));

		
		auto pdbPath = std::filesystem::path(path).parent_path().wstring() + L"/" + debugDataPath->GetStringPointer();
		std::ofstream out(pdbPath, std::ios::out | std::ios::binary);
		if (out.is_open())
		{
			out.write((char*)debugBlob->GetBufferPointer(), debugBlob->GetBufferSize());
			out.flush();
			out.close();
		}
#endif

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

	std::string D3D12Shader::ReadAndPreProcessFile(const std::string& path)
	{
		std::ifstream t(path);
		std::stringstream buffer;
		buffer << t.rdbuf();
		std::string file = buffer.str();

		// TODO: logic below only works if sv_vertexid is declared in arugments, if it's an input struct it won't work
		if (Stage == ShaderStage::Vertex)
		{
			int semantic = file.find("SV_VertexID");
			if (semantic != std::string::npos)
			{
				int colonAfter = file.find_last_of(':', semantic);
				int spaceBefore = file.find_last_of(' ', colonAfter) + 1;
				std::string variableName = file.substr(spaceBefore, colonAfter - spaceBefore);

				file.replace(spaceBefore, colonAfter - spaceBefore, "g_LocalVertexID");
				int colonIndex = file.find("{", colonAfter);

				file.insert(colonIndex + 1, "uint " + variableName + " = g_LocalVertexID + g_SpecialConstants.vertexOffset;");
			}

			semantic = file.find("SV_InstanceID");
			if (semantic != std::string::npos)
			{
				int colonAfter = file.find_last_of(':', semantic);
				int spaceBefore = file.find_last_of(' ', colonAfter) + 1;
				std::string variableName = file.substr(spaceBefore, colonAfter - spaceBefore);

				file.replace(spaceBefore, colonAfter - spaceBefore, "g_LocalInstanceID");
				int colonIndex = file.find("{", colonAfter);

				file.insert(colonIndex + 1, "uint " + variableName + " = g_LocalInstanceID + g_SpecialConstants.instanceOffset;");
			}
		}

		return file;
	}

}