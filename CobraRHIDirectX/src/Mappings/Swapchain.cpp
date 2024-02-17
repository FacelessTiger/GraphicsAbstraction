#include <DirectXRHI.h>

namespace GraphicsAbstraction {

	Ref<Swapchain> Swapchain::Create(const Ref<Window>& window, const glm::vec2& size, bool enableVSync)
	{
		auto swapchain = CreateRef<Swapchain>();
		swapchain->impl = new Impl<Swapchain>(window, size, enableVSync);
		return swapchain;
	}

	Swapchain::~Swapchain()
	{
		impl->Context->GetFrameDeletionQueue().Push(impl->Swapchain);
		delete impl;
	}

	void Swapchain::Resize(uint32_t width, uint32_t height)
	{
		impl->Width = width;
		impl->Height = height;
		for (auto image : impl->Images)
		{
			image->impl->Width = width;
			image->impl->Height = height;
		}

		impl->Dirty = true;
	}

	void Swapchain::SetVsync(bool enabled) { impl->Vsync = enabled; }
	Ref<Image> Swapchain::GetCurrent() { return impl->Images[impl->ImageIndex]; }

	Impl<Swapchain>::Impl(const Ref<Window>& window, const glm::vec2& size, bool enableVSync)
		: Context(Impl<GraphicsContext>::Reference), Width((uint32_t)size.x), Height((uint32_t)size.y), Vsync(enableVSync)
	{
		CreateSwapchain((HWND)window->GetNativeWindow());
	}

	void Impl<Swapchain>::CreateSwapchain(HWND hwnd)
	{
		ComPtr<IDXGIFactory4> factory;
		uint32_t factoryFlags = 0;
#ifndef GA_DIST
		factoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

		D3D12_CHECK(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&factory)));

		DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {
			.Width = Width,
			.Height = Height,
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
			.Stereo = false,
			.SampleDesc = { 1, 0 },
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = 3,
			.Scaling = DXGI_SCALING_STRETCH,
			.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
			.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
			.Flags = (uint32_t)(CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0)
		};

		ComPtr<IDXGISwapChain1> swapchain1;
		D3D12_CHECK(factory->CreateSwapChainForHwnd(Context->GraphicsQueue.Get(), hwnd, &swapchainDesc, nullptr, nullptr, &swapchain1));

		// Disable alt+enter fullscreen toggle
		D3D12_CHECK(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
		D3D12_CHECK(swapchain1.As(&Swapchain));

		// create descriptor heap
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			.NumDescriptors = 3
		};
		D3D12_CHECK(Context->Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&RTVHeap)));

		UpdateRenderTargetViews();
	}

	bool Impl<Swapchain>::CheckTearingSupport()
	{
		BOOL allowTearing = false;

		ComPtr<IDXGIFactory5> factory;
		D3D12_CHECK(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));

		if (FAILED(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(BOOL)))) allowTearing = false;
		return allowTearing == TRUE;
	}

	void Impl<Swapchain>::ResizeImpl()
	{
		for (int i = 0; i < Images.size(); i++) Images[i]->impl->AResource.Resource.Reset();

		DXGI_SWAP_CHAIN_DESC swapchainDesc = {};
		D3D12_CHECK(Swapchain->GetDesc(&swapchainDesc));
		D3D12_CHECK(Swapchain->ResizeBuffers(3, Width, Height, swapchainDesc.BufferDesc.Format, swapchainDesc.Flags));

		UpdateRenderTargetViews();
		Dirty = false;
	}

	void Impl<Swapchain>::UpdateRenderTargetViews()
	{
		auto descriptorSize = Context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		auto rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(RTVHeap->GetCPUDescriptorHandleForHeapStart());

		D3D12_RENDER_TARGET_VIEW_DESC desc = {
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
			.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D
		};

		Images.clear();
		for (int i = 0; i < 3; i++)
		{
			ComPtr<ID3D12Resource> backBuffer;
			D3D12_CHECK(Swapchain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

			Context->Device->CreateRenderTargetView(backBuffer.Get(), &desc, rtvHandle);

			auto image = CreateRef<Image>();
			image->impl = new Impl<Image>(backBuffer, D3D12_RESOURCE_STATE_PRESENT, ImageFormat::R8G8B8A8_UNORM, rtvHandle);
			Images.push_back(image);
			rtvHandle.Offset(1, descriptorSize);
		}
	}

}