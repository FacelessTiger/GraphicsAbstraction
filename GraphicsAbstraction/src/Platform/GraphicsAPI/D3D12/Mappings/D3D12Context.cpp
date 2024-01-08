#include "D3D12Context.h"

#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Queue.h>
#include <d3dx12/d3dx12.h>

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 611; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; }

namespace GraphicsAbstraction {

	using namespace Microsoft::WRL;

	Ref<GraphicsContext> GraphicsContext::s_Instance;

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
		s_Instance = CreateRef<D3D12Context>(frameInFlightCount);
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
		SetupBindless();

		PipelineManager = new D3D12PipelineManager(*this);
		FrameDeletionQueues.resize(frameInFlightCount, *this);
	}

	D3D12Context::~D3D12Context()
	{
		GraphicsQueue.Reset();

		for (auto& deletionQueue : FrameDeletionQueues)
			deletionQueue.Flush();

		delete PipelineManager;
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

	void D3D12Context::SetupBindless()
	{
		CD3DX12_ROOT_PARAMETER1 pushConstant;
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC descVersion;
		pushConstant.InitAsConstants(128 / 4, 0);
		descVersion.Init_1_1(1, &pushConstant, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED | D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED);

		ComPtr<ID3DBlob> signature, error;
		D3D12_CHECK(D3DX12SerializeVersionedRootSignature(&descVersion, D3D_ROOT_SIGNATURE_VERSION_1_1, &signature, &error));
		D3D12_CHECK(Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&BindlessRootSignature)));

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			.NumDescriptors = 1'000'000,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		};
		D3D12_CHECK(Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&BindlessDescriptorHeap)));

		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		heapDesc.NumDescriptors = 2000;
		D3D12_CHECK(Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&BindlessSamplerHeap)));
	}

}