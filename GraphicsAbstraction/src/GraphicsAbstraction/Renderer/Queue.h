#pragma once

#include <memory>

#include <GraphicsAbstraction/Core/Log.h>

namespace GraphicsAbstraction {

	class Swapchain;
	class CommandBuffer;
	class Fence;

	enum class QueueType
	{
		Graphics
	};

	class Queue
	{
	public:
		virtual ~Queue() = default;

		virtual void Acquire(const std::shared_ptr<Swapchain>& swapchain, const std::shared_ptr<Fence>& fence) = 0;
		virtual void Submit(const std::shared_ptr<CommandBuffer>& cmd, const std::shared_ptr<Fence>& wait, const std::shared_ptr<Fence>& signal) = 0;
		virtual void Present(const std::shared_ptr<Swapchain>& swapchain, const std::shared_ptr<Fence>& wait) = 0;
	};

}