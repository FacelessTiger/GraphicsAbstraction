#pragma once

#include <GraphicsAbstraction/Renderer/GraphicsContext.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <comdef.h>
#include <codecvt>
#include <wrl.h>

#ifndef GA_DIST
	#define D3D12_CHECK(x)																										\
		do																														\
		{																														\
			HRESULT err = x;																									\
			if (FAILED(err))																									\
			{																													\
				_com_error errMessage(err);																						\
				GA_CORE_ASSERT(false, std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(errMessage.ErrorMessage()))	\
			}																													\
		} while (0)
#else
	#define D3D12_CHECK(x) x
#endif

namespace GraphicsAbstraction {

	class D3D12Context : public GraphicsContext
	{
	public:
		Microsoft::WRL::ComPtr<ID3D12Device2> Device;
		DWORD DebugCallbackCookie;
	public:
		D3D12Context(uint32_t frameInFlightCount);
		void ShutdownImpl() override { };

		Ref<Queue> GetQueueImpl(QueueType type) override { return nullptr; }
		void SetFrameInFlightImpl(uint32_t fif) override { };
	private:
		Microsoft::WRL::ComPtr<IDXGIAdapter4> SetupAdapter();
		void SetupDevice(Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter);
	};

}