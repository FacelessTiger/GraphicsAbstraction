#pragma once

#include <GraphicsAbstraction/Renderer/CommandPool.h>

#include <vulkan/vulkan.h>

namespace GraphicsAbstraction {

	class VulkanContext;

	class VulkanCommandPool : public CommandPool
	{
	public:
		VulkanCommandPool(std::shared_ptr<GraphicsContext> context, QueueType type);
		virtual ~VulkanCommandPool();

		std::shared_ptr<CommandBuffer> CreateCommandBuffer() const override;
		CommandPoolBuffers CreateCommandBuffers(uint32_t count) const override;
	private:
		void InitQueues(QueueType type);

		void CreateVulkanCommandBuffers(uint32_t count, VkCommandBuffer* data) const;
	private:
		VkCommandPool m_CommandPool;
		std::shared_ptr<VulkanContext> m_Context;

		VkQueue m_Queue;
		uint32_t m_QueueFamily;
	};

}