#pragma once

#include <GraphicsAbstraction/Renderer/GraphicsContext.h>
#include <Platform/GraphicsAPI/D3D12/InternalManagers/D3D12PipelineManager.h>
#include <Platform/GraphicsAPI/D3D12/InternalManagers/D3D12DeletionQueue.h>

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
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> GraphicsQueue;

		Microsoft::WRL::ComPtr<ID3D12RootSignature> BindlessRootSignature;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> BindlessDescriptorHeap, BindlessSamplerHeap;

		D3D12PipelineManager* PipelineManager;
		std::vector<D3D12DeletionQueue> FrameDeletionQueues;
		uint32_t FrameInFlight = 0;
	public:
		D3D12Context(uint32_t frameInFlightCount);
		~D3D12Context();

		Ref<Queue> GetQueueImpl(QueueType type) override;
		void SetFrameInFlightImpl(uint32_t fif) override { FrameInFlight = fif; FrameDeletionQueues[FrameInFlight].Flush(); };

		inline D3D12DeletionQueue& GetFrameDeletionQueue() { return FrameDeletionQueues[FrameInFlight]; }
		inline static Ref<D3D12Context> GetReference() { return { s_Instance, (D3D12Context*)s_Instance.Get() }; }
	private:
		Microsoft::WRL::ComPtr<IDXGIAdapter4> SetupAdapter();
		void SetupDevice(Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter);
		void SetupBindless();
	};

}