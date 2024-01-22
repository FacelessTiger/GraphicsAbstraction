#pragma once

#include <memory>
#include <GraphicsAbstraction/Core/Core.h>

namespace GraphicsAbstraction {

	struct Swapchain;
	struct CommandList;
	struct Fence;

	enum class QueueType
	{
		Graphics
	};

	struct GA_DLL_LINK Queue : public RefCounted
	{
		Impl<Queue>* impl;
		virtual ~Queue();

		void Acquire(const Ref<Swapchain>& swapchain, const Ref<Fence>& fence);
		void Submit(const Ref<CommandList>& cmd, const Ref<Fence>& wait, const Ref<Fence>& signal);
		void Present(const Ref<Swapchain>& swapchain, const Ref<Fence>& wait);
	};

}