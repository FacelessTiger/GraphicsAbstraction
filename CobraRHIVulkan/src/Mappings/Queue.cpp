#include <VulkanRHI.h>

namespace GraphicsAbstraction {

	Queue::~Queue()
	{
		delete impl;
	}

	void Queue::Acquire(const Ref<Swapchain>& swapchain, const Ref<Fence>& fence)
	{
		if (swapchain->impl->Dirty) swapchain->impl->Recreate();
		vkAcquireNextImageKHR(impl->Context.Device, swapchain->impl->Swapchain, UINT64_MAX, swapchain->impl->Semaphores[swapchain->impl->SemaphoreIndex], nullptr, &swapchain->impl->ImageIndex);

		uint64_t signalValue = ++fence->impl->Value;
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
			.pWaitSemaphores = &swapchain->impl->Semaphores[swapchain->impl->SemaphoreIndex],
			.pWaitDstStageMask = &stageMask,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &fence->impl->TimelineSemaphore
		};
		swapchain->impl->SemaphoreIndex = (swapchain->impl->SemaphoreIndex + 1) % swapchain->impl->Semaphores.size();

		VK_CHECK(vkQueueSubmit(impl->Queue, 1, &submitInfo, nullptr));
	}

	void Queue::Submit(const Ref<CommandList>& cmd, const Ref<Fence>& wait, const Ref<Fence>& signal)
	{
		vkEndCommandBuffer(cmd->impl->CommandBuffer);
		VkTimelineSemaphoreSubmitInfo timelineInfo = { .sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO };

		VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		VkSubmitInfo submitInfo = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = &timelineInfo,
			.commandBufferCount = 1,
			.pCommandBuffers = &cmd->impl->CommandBuffer
		};

		uint64_t waitValue;
		uint64_t signalValue;

		if (wait)
		{
			waitValue = wait->impl->Value;
			timelineInfo.waitSemaphoreValueCount = 1;
			timelineInfo.pWaitSemaphoreValues = &waitValue;

			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = &wait->impl->TimelineSemaphore;
			submitInfo.pWaitDstStageMask = &stageMask;
		}

		if (signal)
		{
			signalValue = ++signal->impl->Value;
			timelineInfo.signalSemaphoreValueCount = 1;
			timelineInfo.pSignalSemaphoreValues = &signalValue;

			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &signal->impl->TimelineSemaphore;
		}

		VK_CHECK(vkQueueSubmit(impl->Queue, 1, &submitInfo, nullptr));
	}

	void Queue::Present(const Ref<Swapchain>& swapchain, const Ref<Fence>& wait)
	{
		VkSemaphore* binaryWaits = nullptr;

		VkTimelineSemaphoreSubmitInfo timelineInfo = {
			.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
			.waitSemaphoreValueCount = 1,
			.pWaitSemaphoreValues = &wait->impl->Value
		};

		VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		VkSubmitInfo submitInfo = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = &timelineInfo,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &wait->impl->TimelineSemaphore,
			.pWaitDstStageMask = &stageMask,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &swapchain->impl->Semaphores[swapchain->impl->SemaphoreIndex]
		};
		VK_CHECK(vkQueueSubmit(impl->Queue, 1, &submitInfo, nullptr));

		binaryWaits = &swapchain->impl->Semaphores[swapchain->impl->SemaphoreIndex];
		swapchain->impl->SemaphoreIndex = (swapchain->impl->SemaphoreIndex + 1) % swapchain->impl->Semaphores.size();

		VkPresentInfoKHR presentInfo = {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = binaryWaits,
			.swapchainCount = 1,
			.pSwapchains = &swapchain->impl->Swapchain,
			.pImageIndices = &swapchain->impl->ImageIndex
		};
		VK_CHECK(vkQueuePresentKHR(impl->Queue, &presentInfo));
	}

	Impl<Queue>::Impl(Impl<GraphicsContext>& context, VkQueue queue, uint32_t queueFamily)
		: Context(context), Queue(queue), QueueFamily(queueFamily)
	{ }

}