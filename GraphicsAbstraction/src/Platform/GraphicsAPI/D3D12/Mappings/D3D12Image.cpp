#include "D3D12Image.h"

#include <d3dx12/d3dx12.h>

namespace GraphicsAbstraction {

	Ref<Image> Image::Create(const glm::vec2& size, ImageFormat format, ImageUsage usage)
	{
		return CreateRef<D3D12Image>(size, format, usage);
	}

	D3D12Image::D3D12Image(const glm::vec2& size, ImageFormat format, ImageUsage usage)
		: m_Context(D3D12Context::GetReference())
	{
		// TODO: need to be modified for generic images
		auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, size.x, size.y, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
		D3D12_CLEAR_VALUE depthClear = { .Format = DXGI_FORMAT_D32_FLOAT };

		D3D12_CHECK(m_Context.Device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClear, IID_PPV_ARGS(&Image)));

		// TODO: definitely depth stencil specific
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

	D3D12Image::D3D12Image(Microsoft::WRL::ComPtr<ID3D12Resource> image, D3D12_RESOURCE_STATES state, ImageFormat format, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)
		: m_Context(D3D12Context::GetReference()), Image(image), State(state), Format(format), CpuHandle(cpuHandle)
	{ }

	void D3D12Image::TransitionState(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_RESOURCE_STATES newState)
	{
		//CD3DX12_RESOURCE_BARRIER barrier;
		//barrier.Transition()

		D3D12_RESOURCE_BARRIER barrier = { .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION };
		barrier.Transition.pResource = Image.Get();
		barrier.Transition.StateBefore = State;
		barrier.Transition.StateAfter = newState;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		commandList->ResourceBarrier(1, &barrier);
		State = newState;
	}

}