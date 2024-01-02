#include "D3D12Queue.h"

#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12CommandBuffer.h>
#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Fence.h>
#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Swapchain.h>

namespace GraphicsAbstraction {

	D3D12Queue::D3D12Queue(Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue, D3D12_COMMAND_LIST_TYPE type)
		: Queue(queue), Type(type)
	{
		m_DeletionFence = CreateRef<D3D12Fence>();
	}

	D3D12Queue::~D3D12Queue()
	{
		Queue->Signal(m_DeletionFence->Fence.Get(), 1);
		m_DeletionFence->Wait();
	}

	void D3D12Queue::Acquire(const Ref<Swapchain>& swapchain, const Ref<Fence>& fence)
	{
		auto& d3d12Swapchain = (D3D12Swapchain&)*swapchain;
		auto& d3d12Fence = (D3D12Fence&)*fence;
		
		d3d12Swapchain.ImageIndex = d3d12Swapchain.Swapchain->GetCurrentBackBufferIndex();
		D3D12_CHECK(Queue->Signal(d3d12Fence.Fence.Get(), ++d3d12Fence.Value));
	}

	void D3D12Queue::Submit(const Ref<CommandBuffer>& cmd, const Ref<Fence>& wait, const Ref<Fence>& signal)
	{
		auto& d3d12Cmd = (D3D12CommandBuffer&)*cmd;
		auto& d3d12Wait = (D3D12Fence&)*wait;
		auto& d3d12Signal = (D3D12Fence&)*signal;

		d3d12Cmd.CommandList->Close();
		D3D12_CHECK(Queue->Wait(d3d12Wait.Fence.Get(), d3d12Wait.Value));

		ID3D12CommandList* list = d3d12Cmd.CommandList.Get();
		Queue->ExecuteCommandLists(1, &list);

		D3D12_CHECK(Queue->Signal(d3d12Signal.Fence.Get(), ++d3d12Signal.Value));
	}

	void D3D12Queue::Present(const Ref<Swapchain>& swapchain, const Ref<Fence>& wait)
	{
		auto& d3d12Swapchain = (D3D12Swapchain&)*swapchain;
		auto& d3d12Wait = (D3D12Fence&)*wait;

		d3d12Wait.Wait();
		d3d12Swapchain.Swapchain->Present(d3d12Swapchain.Vsync ? 1 : 0, !d3d12Swapchain.Vsync ? DXGI_PRESENT_ALLOW_TEARING : 0);
	}

}