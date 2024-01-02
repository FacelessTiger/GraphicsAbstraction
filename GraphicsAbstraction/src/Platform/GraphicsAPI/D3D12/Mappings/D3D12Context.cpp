#include "D3D12Context.h"

#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Queue.h>

namespace GraphicsAbstraction {

	using namespace Microsoft::WRL;

	Scope<GraphicsContext> GraphicsContext::s_Instance;

	// TODO: DOESNT DO ANYTHING CAUSE DIRECTX SUCKS GRRRRRR
	static void debugCallback(D3D12_MESSAGE_CATEGORY category, D3D12_MESSAGE_SEVERITY severity, D3D12_MESSAGE_ID id, LPCSTR pDescription, void* pContext)
	{
		switch (severity)
		{
			case D3D12_MESSAGE_SEVERITY_ERROR: GA_CORE_ERROR("{0}", pDescription); break;
		}
	}

	void GraphicsContext::Init(uint32_t frameInFlightCount)
	{
		s_Instance = CreateScope<D3D12Context>(frameInFlightCount);
	}

	D3D12Context::D3D12Context(uint32_t frameInFlightCount)
	{
#ifndef GA_DIST
		ComPtr<ID3D12Debug> debugInterface;
		D3D12_CHECK(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
		debugInterface->EnableDebugLayer();
#endif

		auto adapter = SetupAdapter();
		SetupDevice(adapter);
	}

	Ref<Queue> D3D12Context::GetQueueImpl(QueueType type)
	{
		switch (type)
		{
			case QueueType::Graphics: return CreateRef<D3D12Queue>(GraphicsQueue, D3D12_COMMAND_LIST_TYPE_DIRECT);
		}

		GA_CORE_ASSERT(false, "Unknown queue type!");
		return nullptr;
	}

	ComPtr<IDXGIAdapter4> D3D12Context::SetupAdapter()
	{
		ComPtr<IDXGIFactory4> dxgiFactory;
		uint32_t createFactoryFlags = 0;
#ifndef GA_DIST
		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

		D3D12_CHECK(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

		ComPtr<IDXGIAdapter1> dxgiAdapter1;
		ComPtr<IDXGIAdapter4> dxgiAdapter4;

		size_t maxDedicatedVideoMemory = 0;
		for (uint32_t i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; i++)
		{
			DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
			dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

			if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
				SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) &&
				dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
			{
				maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
				D3D12_CHECK(dxgiAdapter1.As(&dxgiAdapter4));
			}
		}

		return dxgiAdapter4;
	}

	void D3D12Context::SetupDevice(ComPtr<IDXGIAdapter4> adapter)
	{
		D3D12_CHECK(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&Device)));

#ifndef GA_DIST
		ComPtr<ID3D12InfoQueue> pInfoQueue;
		if (SUCCEEDED(Device.As(&pInfoQueue)))
		{
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

			D3D12_MESSAGE_SEVERITY infoSeverity = D3D12_MESSAGE_SEVERITY_INFO;

			D3D12_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumSeverities = 1;
			filter.DenyList.pSeverityList = &infoSeverity;
			//D3D12_CHECK(pInfoQueue->PushStorageFilter(&filter));
		}
#endif

		D3D12_COMMAND_QUEUE_DESC graphicsQueueDesc = {
			.Type = D3D12_COMMAND_LIST_TYPE_DIRECT
		};
		D3D12_CHECK(Device->CreateCommandQueue(&graphicsQueueDesc, IID_PPV_ARGS(&GraphicsQueue)));
	}

}