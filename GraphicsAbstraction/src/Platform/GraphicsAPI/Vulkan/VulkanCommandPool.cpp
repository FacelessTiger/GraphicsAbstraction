#include "VulkanCommandPool.h"

#include <Platform/GraphicsAPI/Vulkan/VulkanContext.h>
#include <Platform/GraphicsAPI/Vulkan/VulkanCommandBuffer.h>

#include <iostream>

namespace GraphicsAbstraction {

	VulkanCommandPool::VulkanCommandPool(std::shared_ptr<GraphicsContext> context, QueueType type)
	{
		m_Context = std::dynamic_pointer_cast<VulkanContext>(context);

		VkCommandPoolCreateInfo commandPoolInfo = {};
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.pNext = nullptr;
		commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		
		switch (type)
		{
			case QueueType::Graphics:	commandPoolInfo.queueFamilyIndex = m_Context->GetGraphicsQueueFamily(); break;
			default:					std::cerr << "Unknown queue type!" << std::endl; abort();
		}

		VK_CHECK(vkCreateCommandPool(m_Context->GetLogicalDevice(), &commandPoolInfo, nullptr, &m_CommandPool));
		m_Context->PushToDeletionQueue([=] {
			vkDestroyCommandPool(m_Context->GetLogicalDevice(), m_CommandPool, nullptr);
		});
	}

	std::shared_ptr<CommandBuffer> VulkanCommandPool::CreateCommandBuffer() const
	{
		VkCommandBuffer buffer;
		CreateVulkanCommandBuffers(1, &buffer);

		return std::make_shared<VulkanCommandBuffer>(buffer);
	}

	CommandPoolBuffers VulkanCommandPool::CreateCommandBuffers(uint32_t count) const
	{
		std::vector<VkCommandBuffer> vulkanBuffers(count);
		CommandPoolBuffers buffers;
		buffers.reserve(count);

		CreateVulkanCommandBuffers(count, vulkanBuffers.data());
		for (VkCommandBuffer& vulkanBuffer : vulkanBuffers)
			buffers.emplace_back(std::make_shared<VulkanCommandBuffer>(vulkanBuffer));

		return buffers;
	}

	void VulkanCommandPool::CreateVulkanCommandBuffers(uint32_t count, VkCommandBuffer* data) const
	{
		VkCommandBufferAllocateInfo cmdAllocInfo = {};
		cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdAllocInfo.pNext = nullptr;
		cmdAllocInfo.commandPool = m_CommandPool;
		cmdAllocInfo.commandBufferCount = count;
		cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		VK_CHECK(vkAllocateCommandBuffers(m_Context->GetLogicalDevice(), &cmdAllocInfo, data));
	}

}