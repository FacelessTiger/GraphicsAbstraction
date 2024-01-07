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

			return ret;
		}

	}

	Ref<Image> Image::Create(const glm::vec2& size, ImageFormat format, ImageUsage usage)
	{
		return CreateRef<D3D12Image>(size, format, usage);
	}

	D3D12Image::D3D12Image(const glm::vec2& size, ImageFormat format, ImageUsage usage)
		: m_Context(D3D12Context::GetReference()), Width((uint32_t)size.x), Height((uint32_t)size.y), Format(format)
	{
		DXGI_FORMAT d3d12Format = Utils::GAImageFormatToD3D12(format);

		D3D12_RESOURCE_FLAGS resourceFlags = D3D12_RESOURCE_FLAG_NONE;
		D3D12_CLEAR_VALUE depthClear = { .Format = d3d12Format };
		D3D12_CLEAR_VALUE* clearValue = nullptr;
		if (usage & ImageUsage::DepthStencilAttachment)
		{
			resourceFlags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			clearValue = &depthClear;
		}

		auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(d3d12Format, Width, Height, 1, 0, 1, 0, resourceFlags);

		State = Utils::GAImageUsageToD3D12(usage);
		D3D12_CHECK(m_Context.Device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, State, clearValue, IID_PPV_ARGS(&Image)));

		if (usage & ImageUsage::DepthStencilAttachment)
		{
			D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
				.NumDescriptors = 1,
			};
			D3D12_CHECK(m_Context.Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_Heap)));
			CpuHandle = m_Heap->GetCPUDescriptorHandleForHeapStart();

			D3D12_DEPTH_STENCIL_VIEW_DESC depthDesc = {
				.Format = DXGI_FORMAT_D32_FLOAT,
				.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
			};
			m_Context.Device->CreateDepthStencilView(Image.Get(), &depthDesc, CpuHandle);
		}

		if (usage & ImageUsage::Sampled)
		{
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_Context.BindlessDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), Handle.GetValue(), m_Context.Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

			D3D12_SHADER_RESOURCE_VIEW_DESC shaderViewDesc = {
				.Format = d3d12Format,
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Texture2D = {
					.MipLevels = 1
				}
			};
			m_Context.Device->CreateShaderResourceView(Image.Get(), &shaderViewDesc, handle);
		}
	}

	D3D12Image::D3D12Image(Microsoft::WRL::ComPtr<ID3D12Resource> image, D3D12_RESOURCE_STATES state, ImageFormat format, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)
		: m_Context(D3D12Context::GetReference()), Image(image), State(state), Format(format), CpuHandle(cpuHandle)
	{ }

	void D3D12Image::TransitionState(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_RESOURCE_STATES newState)
	{
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(Image.Get(), State, newState, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

		commandList->ResourceBarrier(1, &barrier);
		State = newState;
	}

}