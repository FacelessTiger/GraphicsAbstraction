#include <DirectXRHI.h>

namespace GraphicsAbstraction {

	namespace Utils {

		D3D12_RESOURCE_STATES GAImageUsageToD3D12(ImageUsage usage)
		{
			D3D12_RESOURCE_STATES ret = D3D12_RESOURCE_STATE_COMMON;

			// TODO: color attachment and storage
			if (usage & ImageUsage::DepthStencilAttachment)	ret |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
			//if (usage & ImageUsage::TransferSrc)			ret |= D3D12_RESOURCE_STATE_COPY_SOURCE;
			//if (usage & ImageUsage::TransferDst)			ret |= D3D12_RESOURCE_STATE_COPY_DEST;
			if (usage & ImageUsage::Sampled)				ret |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			if (usage & ImageUsage::ColorAttachment)		ret = D3D12_RESOURCE_STATE_RENDER_TARGET;

			return ret;
		}

		D3D12_RESOURCE_FLAGS GAImageUsageToD3D12Flags(ImageUsage usage)
		{
			D3D12_RESOURCE_FLAGS ret = D3D12_RESOURCE_FLAG_NONE;

			if (usage & ImageUsage::DepthStencilAttachment)	ret |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			if (usage & ImageUsage::ColorAttachment)		ret |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			if (usage & ImageUsage::Storage)				ret |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

			return ret;
		}

	}

	Ref<Image> Image::Create(const glm::vec2& size, ImageFormat format, ImageUsage usage)
	{
		auto image = CreateRef<Image>();
		image->impl = new Impl<Image>(size, format, usage);
		return image;
	}

	Image::~Image()
	{
		impl->Context->GetFrameDeletionQueue().Push(impl->Image);
		delete impl;
	}

	void Image::Resize(const glm::vec2& size)
	{
		impl->Width = (uint32_t)size.x;
		impl->Height = (uint32_t)size.y;

		impl->Context->GetFrameDeletionQueue().Push(impl->Image);
		impl->Create();
	}

	uint32_t Image::GetSampledHandle() const { return impl->Handle.GetValue(); }
	uint32_t Image::GetStorageHandle() const { return impl->Handle.GetValue(); }
	uint32_t Image::GetWidth() const { return impl->Width; }
	uint32_t Image::GetHeight() const { return impl->Height; }
	glm::vec2 Image::GetSize() const { return glm::vec2(impl->Width, impl->Height); }

	Impl<Image>::Impl(const glm::vec2& size, ImageFormat format, ImageUsage usage)
		: Context(Impl<GraphicsContext>::Reference), Width((uint32_t)size.x), Height((uint32_t)size.y), Usage(usage), Format(format)
	{
		Create();
	}

	Impl<Image>::Impl(ComPtr<ID3D12Resource> image, D3D12_RESOURCE_STATES state, ImageFormat format, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)
		: Context(Impl<GraphicsContext>::Reference), Image(image), State(state), Format(format), CpuHandle(cpuHandle)
	{ }

	void Impl<Image>::TransitionState(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_RESOURCE_STATES newState)
	{
		if (State == newState) return;
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(Image.Get(), State, newState, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

		commandList->ResourceBarrier(1, &barrier);
		State = newState;
	}

	void Impl<Image>::Create()
	{
		DXGI_FORMAT d3d12Format = Utils::GAImageFormatToD3D12(Format);

		D3D12_CLEAR_VALUE clear = { .Format = d3d12Format };
		D3D12_CLEAR_VALUE* clearPointer = nullptr;
		if (Usage & ImageUsage::DepthStencilAttachment)
		{
			clearPointer = &clear;
		}
		else if (Usage & ImageUsage::ColorAttachment)
		{
			// TODO: hack to silence warning
			clear.Color[0] = 0.0f;
			clear.Color[1] = 0.0f;
			clear.Color[2] = 0.0f;
			clear.Color[3] = 1.0f;
			clearPointer = &clear;
		}

		D3D12MA::ALLOCATION_DESC allocDesc = { .HeapType = D3D12_HEAP_TYPE_DEFAULT };
		auto resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(d3d12Format, Width, Height, 1, 1, 1, 0, Utils::GAImageUsageToD3D12Flags(Usage));

		State = Utils::GAImageUsageToD3D12(Usage);
		D3D12_CHECK(Context->Allocator->CreateResource(&allocDesc, &resourceDesc, State, clearPointer, &Allocation, IID_PPV_ARGS(&Image)));

		CreateViews(d3d12Format);
	}

	void Impl<Image>::CreateViews(DXGI_FORMAT d3d12Format)
	{
		if (Usage & ImageUsage::DepthStencilAttachment)
		{
			D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
				.NumDescriptors = 1,
			};
			D3D12_CHECK(Context->Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&Heap)));
			CpuHandle = Heap->GetCPUDescriptorHandleForHeapStart();

			D3D12_DEPTH_STENCIL_VIEW_DESC depthDesc = {
				.Format = d3d12Format,
				.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
			};
			Context->Device->CreateDepthStencilView(Image.Get(), &depthDesc, CpuHandle);
		}

		if (Usage & ImageUsage::ColorAttachment)
		{
			D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
				.NumDescriptors = 1,
			};
			D3D12_CHECK(Context->Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&Heap)));
			CpuHandle = Heap->GetCPUDescriptorHandleForHeapStart();

			D3D12_RENDER_TARGET_VIEW_DESC desc = {
				.Format = d3d12Format,
				.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D
			};
			Context->Device->CreateRenderTargetView(Image.Get(), &desc, CpuHandle);
		}

		uint32_t incrementSize = Context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		if (Usage & ImageUsage::Sampled)
		{
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(Context->BindlessDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), Handle.GetValue(), incrementSize);

			D3D12_SHADER_RESOURCE_VIEW_DESC shaderViewDesc = {
				.Format = d3d12Format,
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Texture2D = {
					.MipLevels = 1
				}
			};
			Context->Device->CreateShaderResourceView(Image.Get(), &shaderViewDesc, handle);
		}

		if (Usage & ImageUsage::Storage)
		{
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(Context->BindlessDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), Handle.GetValue() + 1, incrementSize);

			D3D12_UNORDERED_ACCESS_VIEW_DESC unorderedViewDesc = {
				.Format = d3d12Format,
				.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D
			};
			Context->Device->CreateUnorderedAccessView(Image.Get(), nullptr, &unorderedViewDesc, handle);
		}
	}

}