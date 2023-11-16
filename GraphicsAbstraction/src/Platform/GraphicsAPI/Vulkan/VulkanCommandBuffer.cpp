#include "VulkanCommandBuffer.h"

#include <Platform/GraphicsAPI/Vulkan/VulkanContext.h>
#include <Platform/GraphicsAPI/Vulkan/VulkanFence.h>
#include <Platform/GraphicsAPI/Vulkan/VulkanSwapchain.h>

namespace GraphicsAbstraction {

	VulkanCommandBuffer::VulkanCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue)
		: m_CommandBuffer(commandBuffer), m_Queue(queue)
	{ }

	void VulkanCommandBuffer::Reset() const
	{
		VK_CHECK(vkResetCommandBuffer(m_CommandBuffer, 0));
	}

	void VulkanCommandBuffer::Submit(std::shared_ptr<Swapchain> swapchain, std::shared_ptr<Fence> fence) const
	{
		std::shared_ptr<VulkanSwapchain> vulkanSwapchain = std::dynamic_pointer_cast<VulkanSwapchain>(swapchain);
		std::shared_ptr<VulkanFence> vulkanFence = std::dynamic_pointer_cast<VulkanFence>(fence);

		VkSemaphore presentSemaphore = vulkanSwapchain->GetPresentSemaphore();
		VkSemaphore renderSemaphore = vulkanSwapchain->GetRenderSemaphore();

		VkSubmitInfo submit = {};
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit.pNext = nullptr;

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submit.pWaitDstStageMask = &waitStage;

		submit.waitSemaphoreCount = 1;
		submit.pWaitSemaphores = &presentSemaphore;
		submit.signalSemaphoreCount = 1;
		submit.pSignalSemaphores = &renderSemaphore;

		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &m_CommandBuffer;

		VK_CHECK(vkQueueSubmit(m_Queue, 1, &submit, vulkanFence->GetInternal()));
	}

	void VulkanCommandBuffer::Present(std::shared_ptr<Swapchain> swapchain, uint32_t swapchainImageIndex) const
	{
		std::shared_ptr<VulkanSwapchain> vulkanSwapchain = std::dynamic_pointer_cast<VulkanSwapchain>(swapchain);

		VkSwapchainKHR vkSwapchain = vulkanSwapchain->GetInternal();
		VkSemaphore renderSemaphore = vulkanSwapchain->GetRenderSemaphore();

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;

		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &vkSwapchain;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderSemaphore;
		presentInfo.pImageIndices = &swapchainImageIndex;

		VK_CHECK(vkQueuePresentKHR(m_Queue, &presentInfo));
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