#pragma once

#include <memory>
#include<vector>

namespace GraphicsAbstraction {

	class GraphicsContext;
	class CommandBuffer;

	enum class QueueType
	{
		Graphics
	};

	using CommandPoolBuffers = std::vector<std::shared_ptr<CommandBuffer>>;

	class CommandPool
	{
	public:
		virtual ~CommandPool() = default;

		virtual std::shared_ptr<CommandBuffer> CreateCommandBuffer() const = 0;
		virtual CommandPoolBuffers CreateCommandBuffers(uint32_t count) const = 0;

		static std::shared_ptr<CommandPool> Create(std::shared_ptr<GraphicsContext> context, QueueType type);
	};

}