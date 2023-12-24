#pragma once

#include <memory>

#include <GraphicsAbstraction/Core/Log.h>

namespace GraphicsAbstraction {

	class GraphicsContext;
	class CommandBuffer;
	class Queue;

	class CommandPool
	{
	public:
		virtual ~CommandPool() = default;

		virtual void Reset() = 0;
		virtual std::shared_ptr<CommandBuffer> Begin() = 0;

		static std::shared_ptr<CommandPool> Create(const std::shared_ptr<Queue>& queue);
	};

}