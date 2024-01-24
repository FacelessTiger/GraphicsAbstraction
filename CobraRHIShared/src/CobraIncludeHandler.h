#pragma once

#include <wrl/client.h>
#include <unordered_set>
#include <string>
#include <dxcapi.h>

namespace GraphicsAbstraction {

	class CobraIncludeHandler : public IDxcIncludeHandler
	{
	public:
		HRESULT STDMETHODCALLTYPE LoadSource(_In_z_ LPCWSTR pFilename, _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource) override;

		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override { return E_NOINTERFACE; }
		ULONG STDMETHODCALLTYPE AddRef(void) override { return 0; }
		ULONG STDMETHODCALLTYPE Release(void) override { return 0; }
	private:
		std::unordered_set<std::wstring> m_IncludedFiles;
	};

}