#pragma once

#include <GraphicsAbstraction/Renderer/Queue.h>

namespace GraphicsAbstraction {

	class D3D12Queue : public Queue
	{
	public:
		void Acquire(const Ref<Swapchain>& swapchain, const Ref<Fence>& fence) override { }
		void Submit(const Ref<CommandBuffer>& cmd, const Ref<Fence>& wait, const Ref<Fence>& signal) override { }
		void Present(const Ref<Swapchain>& swapchain, const Ref<Fence>& wait) override { }
	};

}