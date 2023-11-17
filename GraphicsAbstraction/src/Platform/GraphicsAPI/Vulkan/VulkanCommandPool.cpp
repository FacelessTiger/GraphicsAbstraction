#include "VulkanCommandPool.h"

#include <Platform/GraphicsAPI/Vulkan/VulkanContext.h>
#include <Platform/GraphicsAPI/Vulkan/VulkanCommandBuffer.h>

#include <GraphicsAbstraction/Debug/Instrumentor.h>

namespace GraphicsAbstraction {

	VulkanCommandPool::VulkanCommandPool(std::shared_ptr<GraphicsContext> context, QueueType type)
	{
		GA_PROFILE_SCOPE();

		m_Context = std::dynamic_pointer_cast<VulkanContext>(context);
		InitQueues(type);

		VkCommandPoolCreateInfo commandPoolInfo = {};
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.pNext = nullptr;
		commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolInfo.queueFamilyIndex = m_QueueFamily;

		VK_CHECK(vkCreateCommandPool(m_Context->GetLogicalDevice(), &commandPoolInfo, nullptr, &m_CommandPool));
		m_Context->PushToDeletionQueue([commandPool = m_CommandPool](VulkanContext& context) {
			vkDestroyCommandPool(context.GetLogicalDevice(), commandPool, nullptr);
		});
	}

	std::shared_ptr<CommandBuffer> VulkanCommandPool::CreateCommandBuffer() const
	{
		GA_PROFILE_SCOPE();

		VkCommandBuffer buffer;
		CreateVulkanCommandBuffers(1, &buffer);

		return std::make_shared<VulkanCommandBuffer>(buffer, m_Queue);
	}

	CommandPoolBuffers VulkanCommandPool::CreateCommandBuffers(uint32_t count) const
	{
		GA_PROFILE_SCOPE();

		std::vector<VkCommandBuffer> vulkanBuffers(count);
		CommandPoolBuffers buffers;
		buffers.reserve(count);

		CreateVulkanCommandBuffers(count, vulkanBuffers.data());
		for (VkCommandBuffer& vulkanBuffer : vulkanBuffers)
			buffers.emplace_back(std::make_shared<VulkanCommandBuffer>(vulkanBuffer, m_Queue));

		return buffers;
	}

	void VulkanCommandPool::InitQueues(QueueType type)
	{
		switch (type)
		{
			case QueueType::Graphics:
				m_Queue = m_Context->GetGraphicsQeue();
				m_QueueFamily = m_Context->GetGraphicsQueueFamily(); 
				return;
		}

		GA_CORE_ASSERT(false, "Unknown queue type!");
	}

	void VulkanCommandPool::CreateVulkanCommandBuffers(uint32_t count, VkCommandBuffer* data) const
	{
		GA_PROFILE_SCOPE();

		VkCommandBufferAllocateInfo cmdAllocInfo = {};
		cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdAllocInfo.pNext = nullptr;
		cmdAllocInfo.commandPool = m_CommandPool;
		cmdAllocInfo.commandBufferCount = count;
		cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		VK_CHECK(vkAllocateCommandBuffers(m_Context->GetLogicalDevice(), &cmdAllocInfo, data));
	}

}