#include "VulkanCommandBuffer.h"

#include <Platform/GraphicsAPI/Vulkan/VulkanContext.h>

namespace GraphicsAbstraction {

	VulkanCommandBuffer::VulkanCommandBuffer(VkCommandBuffer commandBuffer)
		: m_CommandBuffer(commandBuffer)
	{ }

	void VulkanCommandBuffer::Reset() const
	{
		VK_CHECK(vkResetCommandBuffer(m_CommandBuffer, 0));
	}

	void VulkanCommandBuffer::Begin() const
	{
		VkCommandBufferBeginInfo cmdBeginInfo = {};
		cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBeginInfo.pNext = nullptr;
		cmdBeginInfo.pInheritanceInfo = nullptr;
		cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VK_CHECK(vkBeginCommandBuffer(m_CommandBuffer, &cmdBeginInfo));
	}

	void VulkanCommandBuffer::End() const
	{
		VK_CHECK(vkEndCommandBuffer(m_CommandBuffer));
	}

}