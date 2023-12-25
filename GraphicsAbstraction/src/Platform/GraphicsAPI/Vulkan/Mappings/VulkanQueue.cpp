#include "VulkanQueue.h"

#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanSwapchain.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanCommandBuffer.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanFence.h>
#include <GraphicsAbstraction/Debug/Instrumentor.h>

namespace GraphicsAbstraction {

	VulkanQueue::VulkanQueue(VulkanContext& context, VkQueue queue, uint32_t queueFamily)
		: m_Context(context), Queue(queue), QueueFamily(queueFamily)
	{ }

	void VulkanQueue::Acquire(const std::shared_ptr<Swapchain>& swapchain, const std::shared_ptr<Fence>& fence)
	{
		GA_PROFILE_SCOPE();

		auto vulkanSwapchain = std::static_pointer_cast<VulkanSwapchain>(swapchain);
		auto vulkanFence = std::static_pointer_cast<VulkanFence>(fence);

		if (vulkanSwapchain->Dirty) vulkanSwapchain->Recreate();
		vkAcquireNextImageKHR(m_Context.Device, vulkanSwapchain->Swapchain, UINT64_MAX, vulkanSwapchain->Semaphores[vulkanSwapchain->SemaphoreIndex], nullptr, &vulkanSwapchain->ImageIndex);

		uint64_t signalValue = ++vulkanFence->Value;
		VkTimelineSemaphoreSubmitInfo timelineInfo = { 
			.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
			.signalSemaphoreValueCount = 1,
			.pSignalSemaphoreValues = &signalValue
		};

		VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		VkSubmitInfo submitInfo = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = &timelineInfo,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &vulkanSwapchain->Semaphores[vulkanSwapchain->SemaphoreIndex],
			.pWaitDstStageMask = &stageMask,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &vulkanFence->TimelineSemaphore
		};
		vulkanSwapchain->SemaphoreIndex = (vulkanSwapchain->SemaphoreIndex + 1) % vulkanSwapchain->Semaphores.size();

		VK_CHECK(vkQueueSubmit(Queue, 1, &submitInfo, nullptr));
	}

	void VulkanQueue::Submit(const std::shared_ptr<CommandBuffer>& cmd, const std::shared_ptr<Fence>& wait, const std::shared_ptr<Fence>& signal)
	{
		GA_PROFILE_SCOPE();

		auto vulkanCommandBuffer = std::static_pointer_cast<VulkanCommandBuffer>(cmd);
		auto vulkanWait = std::static_pointer_cast<VulkanFence>(wait);
		auto vulkanSignal = std::static_pointer_cast<VulkanFence>(signal);

		vkEndCommandBuffer(vulkanCommandBuffer->CommandBuffer);
		VkTimelineSemaphoreSubmitInfo timelineInfo = { .sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO };

		VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		VkSubmitInfo submitInfo = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = &timelineInfo,
			.commandBufferCount = 1,
			.pCommandBuffers = &vulkanCommandBuffer->CommandBuffer
		};

		uint64_t waitValue;
		uint64_t signalValue;

		if (wait)
		{
			waitValue = vulkanWait->Value;
			timelineInfo.waitSemaphoreValueCount = 1;
			timelineInfo.pWaitSemaphoreValues = &waitValue;

			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = &vulkanWait->TimelineSemaphore;
			submitInfo.pWaitDstStageMask = &stageMask;
		}

		if (signal)
		{
			signalValue = ++vulkanSignal->Value;
			timelineInfo.signalSemaphoreValueCount = 1;
			timelineInfo.pSignalSemaphoreValues = &signalValue;

			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &vulkanSignal->TimelineSemaphore;
		}

		VK_CHECK(vkQueueSubmit(Queue, 1, &submitInfo, nullptr));
	}

	void VulkanQueue::Present(const std::shared_ptr<Swapchain>& swapchain, const std::shared_ptr<Fence>& wait)
	{
		GA_PROFILE_SCOPE();

		auto vulkanSwapchain = std::static_pointer_cast<VulkanSwapchain>(swapchain);
		auto vulkanFence = std::static_pointer_cast<VulkanFence>(wait);
		VkSemaphore* binaryWaits = nullptr;

		VkTimelineSemaphoreSubmitInfo timelineInfo = {
			.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
			.waitSemaphoreValueCount = 1,
			.pWaitSemaphoreValues = &vulkanFence->Value
		};

		VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		VkSubmitInfo submitInfo = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = &timelineInfo,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &vulkanFence->TimelineSemaphore,
			.pWaitDstStageMask = &stageMask,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &vulkanSwapchain->Semaphores[vulkanSwapchain->SemaphoreIndex]
		};
		VK_CHECK(vkQueueSubmit(Queue, 1, &submitInfo, nullptr));

		binaryWaits = &vulkanSwapchain->Semaphores[vulkanSwapchain->SemaphoreIndex];
		vulkanSwapchain->SemaphoreIndex = (vulkanSwapchain->SemaphoreIndex + 1) % vulkanSwapchain->Semaphores.size();

		VkPresentInfoKHR presentInfo = {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = binaryWaits,
			.swapchainCount = 1,
			.pSwapchains = &vulkanSwapchain->Swapchain,
			.pImageIndices = &vulkanSwapchain->ImageIndex
		};
		VK_CHECK(vkQueuePresentKHR(Queue, &presentInfo));
	}

}