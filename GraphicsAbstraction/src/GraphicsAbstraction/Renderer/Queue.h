#pragma once

#include <memory>

#include <GraphicsAbstraction/Core/Log.h>
#include <GraphicsAbstraction/Core/Core.h>

namespace GraphicsAbstraction {

	class Swapchain;
	class CommandList;
	class Fence;

	enum class QueueType
	{
		Graphics
	};

	class Queue : public RefCounted
	{
	public:
		virtual ~Queue() = default;

		virtual void Acquire(const Ref<Swapchain>& swapchain, const Ref<Fence>& fence) = 0;
		virtual void Submit(const Ref<CommandList>& cmd, const Ref<Fence>& wait, const Ref<Fence>& signal) = 0;
		virtual void Present(const Ref<Swapchain>& swapchain, const Ref<Fence>& wait) = 0;
	};

}