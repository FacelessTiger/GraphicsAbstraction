#include "DxcCompiler.h"
#include "CobraIncludeHandler.h"

#include <wrl/client.h>
#include <dxcapi.h>

#include <assert.h>
#include <iostream>
#include <filesystem>

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

			assert(false);
			return nullptr;
		}

	}

	using namespace Microsoft::WRL;

#include "CobraShaderSource.inl"
	static bool s_Initialized = false;
	static ComPtr<IDxcUtils> s_Utils;
	static ComPtr<IDxcCompiler3> s_Compiler;
	static ComPtr<CobraIncludeHandler> s_IncludeHandler;

	HRESULT __stdcall GraphicsAbstraction::CobraIncludeHandler::LoadSource(LPCWSTR pFilename, IDxcBlob** ppIncludeSource)
	{
		ComPtr<IDxcBlobEncoding> pEncoding;
		bool includeFile = false;
		const char* encodedString;

		auto fileWithoutExtension = std::filesystem::path(pFilename).filename().wstring();

		if (m_IncludedFiles.contains(pFilename)) encodedString = " ";
		else if (std::wcscmp(L"DescriptorHeap", fileWithoutExtension.c_str()) == 0) encodedString = s_DescriptorHeapSource;
		else if (std::wcscmp(L"Bindless", fileWithoutExtension.c_str()) == 0) encodedString = s_BindlessSource;
		else if (std::wcscmp(L"Utils", fileWithoutExtension.c_str()) == 0) encodedString = s_UtilsSource;
		else if (std::wcscmp(L"Cobra", fileWithoutExtension.c_str()) == 0) encodedString = s_CobraSource;
		else includeFile = true;

		HRESULT hr = S_OK;
		if (includeFile) hr = s_Utils->LoadFile(pFilename, nullptr, pEncoding.GetAddressOf());
		else s_Utils->CreateBlobFromPinned(encodedString, (uint32_t)std::strlen(encodedString), DXC_CP_ACP, pEncoding.GetAddressOf());

		*ppIncludeSource = pEncoding.Detach();
		return hr;
	}

	std::vector<uint32_t> DxcCompiler::CompileSource(const std::string& path, const std::string& source, ShaderStage stage, const std::vector<const wchar_t*>& additionalArguments, const std::vector<const wchar_t*>& defines)
	{
		if (!s_Initialized)
		{
			DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&s_Utils));
			DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&s_Compiler));
			s_IncludeHandler = new CobraIncludeHandler();

			s_Initialized = true;
		}

		ComPtr<IDxcBlobEncoding> sourceBlob;
		if (s_Utils->CreateBlob(source.c_str(), (uint32_t)source.size(), DXC_CP_UTF8, &sourceBlob))
		{
			assert(false && ("Could not open source " + source).c_str());
		}

		std::wstring wstring = std::filesystem::path(path).wstring();
		LPCWSTR filePath = wstring.c_str();

		std::vector<const wchar_t*> arguments = additionalArguments;
		arguments.insert(arguments.end(), {
			filePath, DXC_ARG_WARNINGS_ARE_ERRORS,
			L"-E", L"main",
			L"-T", Utils::ShaderStageToDXC(stage),
			//L"-Zi", L"-Od" // Note: Zi causes debug information
		});

		for (auto define : defines)
		{
			arguments.push_back(L"-D");
			arguments.push_back(define);
		}

		DxcBuffer buffer = {
			.Ptr = sourceBlob->GetBufferPointer(),
			.Size = sourceBlob->GetBufferSize(),
			.Encoding = DXC_CP_ACP,
		};

		ComPtr<IDxcResult> result;
		s_Compiler->Compile(&buffer, arguments.data(), (uint32_t)arguments.size(), s_IncludeHandler.Get(), IID_PPV_ARGS(&result));

		HRESULT status;
		result->GetStatus(&status);
		if (FAILED(status) && result)
		{
			ComPtr<IDxcBlobEncoding> errorBlob;
			result->GetErrorBuffer(&errorBlob);

			std::cerr << (const char*)errorBlob->GetBufferPointer() << std::endl;
			assert(false && (const char*)errorBlob->GetBufferPointer());
		}

		ComPtr<IDxcBlob> resultBlob;
		result->GetResult(&resultBlob);

		std::vector<uint32_t> ret(resultBlob->GetBufferSize() / sizeof(uint32_t));
		memcpy(ret.data(), resultBlob->GetBufferPointer(), resultBlob->GetBufferSize());
		return ret;
	}

}