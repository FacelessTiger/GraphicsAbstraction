#include "D3D12Image.h"

namespace GraphicsAbstraction {

	Ref<Image> Image::Create(const glm::vec2& size, ImageFormat format, ImageUsage usage)
	{
		return CreateRef<D3D12Image>(size, format, usage);
	}

	D3D12Image::D3D12Image(Microsoft::WRL::ComPtr<ID3D12Resource> image, D3D12_RESOURCE_STATES state, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)
		: Image(image), State(state), CpuHandle(cpuHandle)
	{ }

	void D3D12Image::TransitionState(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_RESOURCE_STATES newState)
	{
		D3D12_RESOURCE_BARRIER barrier = { .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION };
		barrier.Transition.pResource = Image.Get();
		barrier.Transition.StateBefore = State;
		barrier.Transition.StateAfter = newState;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		commandList->ResourceBarrier(1, &barrier);
		State = newState;
	}

}