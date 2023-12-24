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

		VkSemaphoreSubmitInfo waitInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = vulkanSwapchain->Semaphores[vulkanSwapchain->SemaphoreIndex]
		};
		vulkanSwapchain->SemaphoreIndex = (vulkanSwapchain->SemaphoreIndex + 1) % vulkanSwapchain->Semaphores.size();

		VkSemaphoreSubmitInfo signalInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = vulkanFence->TimelineSemaphore,
			.value = ++vulkanFence->Value,
			.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
		};

		VkSubmitInfo2 submitInfo = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.waitSemaphoreInfoCount = 1,
			.pWaitSemaphoreInfos = &waitInfo,
			.signalSemaphoreInfoCount = 1,
			.pSignalSemaphoreInfos = &signalInfo
		};
		VK_CHECK(vkQueueSubmit2(Queue, 1, &submitInfo, nullptr));
	}

	void VulkanQueue::Submit(const std::shared_ptr<CommandBuffer>& cmd, const std::shared_ptr<Fence>& wait, const std::shared_ptr<Fence>& signal)
	{
		GA_PROFILE_SCOPE();

		auto vulkanCommandBuffer = std::static_pointer_cast<VulkanCommandBuffer>(cmd);
		auto vulkanWait = std::static_pointer_cast<VulkanFence>(wait);
		auto vulkanSignal = std::static_pointer_cast<VulkanFence>(signal);

		VkCommandBufferSubmitInfo bufferInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
			.commandBuffer = vulkanCommandBuffer->CommandBuffer
		};
		vkEndCommandBuffer(vulkanCommandBuffer->CommandBuffer);

		std::vector<VkSemaphoreSubmitInfo> waitInfos;
		if (wait)
		{
			waitInfos.push_back({
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
				.semaphore = vulkanWait->TimelineSemaphore,
				.value = vulkanWait->Value,
				.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
			});
		}

		VkSemaphoreSubmitInfo signalInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = vulkanSignal->TimelineSemaphore,
			.value = ++vulkanSignal->Value,
			.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
		};

		VkSubmitInfo2 submitInfo = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.waitSemaphoreInfoCount = (uint32_t)waitInfos.size(),
			.pWaitSemaphoreInfos = waitInfos.data(),
			.commandBufferInfoCount = 1,
			.pCommandBufferInfos = &bufferInfo,
			.signalSemaphoreInfoCount = 1,
			.pSignalSemaphoreInfos = &signalInfo
		};
		VK_CHECK(vkQueueSubmit2(Queue, 1, &submitInfo, nullptr));
	}

	void VulkanQueue::Present(const std::shared_ptr<Swapchain>& swapchain, const std::shared_ptr<Fence>& wait)
	{
		GA_PROFILE_SCOPE();

		auto vulkanSwapchain = std::static_pointer_cast<VulkanSwapchain>(swapchain);
		auto vulkanFence = std::static_pointer_cast<VulkanFence>(wait);
		VkSemaphore* binaryWaits = nullptr;

		VkSemaphoreSubmitInfo waitInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = vulkanFence->TimelineSemaphore,
			.value = vulkanFence->Value,
			.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
		};

		VkSemaphoreSubmitInfo signalInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = vulkanSwapchain->Semaphores[vulkanSwapchain->SemaphoreIndex]
		};

		VkSubmitInfo2 submitInfo = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.waitSemaphoreInfoCount = 1,
			.pWaitSemaphoreInfos = &waitInfo,
			.signalSemaphoreInfoCount = 1,
			.pSignalSemaphoreInfos = &signalInfo
		};
		VK_CHECK(vkQueueSubmit2(Queue, 1, &submitInfo, nullptr));

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