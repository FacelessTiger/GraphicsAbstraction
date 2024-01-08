#include "D3D12Image.h"

#include <d3dx12/d3dx12.h>
#include <Platform/GraphicsAPI/D3D12/InternalManagers/D3D12Utils.h>

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
		return CreateRef<D3D12Image>(size, format, usage);
	}

	D3D12Image::D3D12Image(const glm::vec2& size, ImageFormat format, ImageUsage usage)
		: m_Context(D3D12Context::GetReference()), Width((uint32_t)size.x), Height((uint32_t)size.y), Usage(usage), Format(format)
	{
		Create();
	}

	D3D12Image::D3D12Image(Microsoft::WRL::ComPtr<ID3D12Resource> image, D3D12_RESOURCE_STATES state, ImageFormat format, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)
		: m_Context(D3D12Context::GetReference()), Image(image), State(state), Format(format), CpuHandle(cpuHandle)
	{ }

	D3D12Image::~D3D12Image()
	{
		m_Context->GetFrameDeletionQueue().Push(Image);
	}

	void D3D12Image::Resize(const glm::vec2& size)
	{
		Width = (uint32_t)size.x;
		Height = (uint32_t)size.y;

		m_Context->GetFrameDeletionQueue().Push(Image);
		Create();
	}

	void D3D12Image::TransitionState(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_RESOURCE_STATES newState)
	{
		if (State == newState) return;
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(Image.Get(), State, newState, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

		commandList->ResourceBarrier(1, &barrier);
		State = newState;
	}

	void D3D12Image::Create()
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

		auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(d3d12Format, Width, Height, 1, 1, 1, 0, Utils::GAImageUsageToD3D12Flags(Usage));

		State = Utils::GAImageUsageToD3D12(Usage);
		D3D12_CHECK(m_Context->Device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, State, clearPointer, IID_PPV_ARGS(&Image)));

		CreateViews(d3d12Format);
	}

	void D3D12Image::CreateViews(DXGI_FORMAT d3d12Format)
	{
		if (Usage & ImageUsage::DepthStencilAttachment)
		{
			D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
				.NumDescriptors = 1,
			};
			D3D12_CHECK(m_Context->Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_Heap)));
			CpuHandle = m_Heap->GetCPUDescriptorHandleForHeapStart();

			D3D12_DEPTH_STENCIL_VIEW_DESC depthDesc = {
				.Format = d3d12Format,
				.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
			};
			m_Context->Device->CreateDepthStencilView(Image.Get(), &depthDesc, CpuHandle);
		}

		if (Usage & ImageUsage::ColorAttachment)
		{
			D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
				.NumDescriptors = 1,
			};
			D3D12_CHECK(m_Context->Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_Heap)));
			CpuHandle = m_Heap->GetCPUDescriptorHandleForHeapStart();

			D3D12_RENDER_TARGET_VIEW_DESC desc = {
				.Format = d3d12Format,
				.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D
			};
			m_Context->Device->CreateRenderTargetView(Image.Get(), &desc, CpuHandle);
		}

		uint32_t incrementSize = m_Context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		if (Usage & ImageUsage::Sampled)
		{
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_Context->BindlessDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), Handle.GetValue(), incrementSize);

			D3D12_SHADER_RESOURCE_VIEW_DESC shaderViewDesc = {
				.Format = d3d12Format,
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Texture2D = {
					.MipLevels = 1
				}
			};
			m_Context->Device->CreateShaderResourceView(Image.Get(), &shaderViewDesc, handle);
		}

		if (Usage & ImageUsage::Storage)
		{
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_Context->BindlessDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), Handle.GetValue() + 1, incrementSize);

			D3D12_UNORDERED_ACCESS_VIEW_DESC unorderedViewDesc = {
				.Format = d3d12Format,
				.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D
			};
			m_Context->Device->CreateUnorderedAccessView(Image.Get(), nullptr, &unorderedViewDesc, handle);
		}
	}

}