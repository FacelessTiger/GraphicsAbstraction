#include <DirectXRHI.h>

namespace GraphicsAbstraction {

	Queue::~Queue()
	{
		impl->Queue->Signal(impl->DeletionFence->impl->Fence.Get(), 1);
		impl->DeletionFence->Wait();

		delete impl;
	}

	void Queue::Acquire(const Ref<Swapchain>& swapchain, const Ref<Fence>& fence)
	{
		if (swapchain->impl->Dirty)
		{
			// flush gpu events
			D3D12_CHECK(impl->Queue->Signal(fence->impl->Fence.Get(), ++fence->impl->Value));
			fence->Wait();

			// do resize
			swapchain->impl->ResizeImpl();
		}
		
		swapchain->impl->ImageIndex = swapchain->impl->Swapchain->GetCurrentBackBufferIndex();
		D3D12_CHECK(impl->Queue->Signal(fence->impl->Fence.Get(), ++fence->impl->Value));
	}

	void Queue::Submit(const Ref<CommandList>& cmd, const Ref<Fence>& wait, const Ref<Fence>& signal)
	{
		cmd->impl->CommandList->Close();
		if (wait) D3D12_CHECK(impl->Queue->Wait(wait->impl->Fence.Get(), wait->impl->Value));

		ID3D12CommandList* list = cmd->impl->CommandList.Get();
		impl->Queue->ExecuteCommandLists(1, &list);

		if (signal) D3D12_CHECK(impl->Queue->Signal(signal->impl->Fence.Get(), ++signal->impl->Value));
	}

	void Queue::Present(const Ref<Swapchain>& swapchain, const Ref<Fence>& wait)
	{
		wait->Wait();
		if (swapchain->impl->Vsync) swapchain->impl->Swapchain->Present(1, 0);
		else swapchain->impl->Swapchain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
	}

	Impl<Queue>::Impl(ComPtr<ID3D12CommandQueue> queue, D3D12_COMMAND_LIST_TYPE type)
		: Queue(queue), Type(type)
	{
		DeletionFence = Fence::Create();
	}

}