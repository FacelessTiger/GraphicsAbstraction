#include "VulkanFence.h"

#include <GraphicsAbstraction/Debug/Instrumentor.h>

namespace GraphicsAbstraction {

	VulkanFence::VulkanFence()
		: m_Context(VulkanContext::GetReference())
	{
		VkSemaphoreTypeCreateInfo timelineCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
			.pNext = nullptr,
			.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
			.initialValue = Value
		};

		VkSemaphoreCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = &timelineCreateInfo
		};
		
		VK_CHECK(vkCreateSemaphore(m_Context->Device, &createInfo, nullptr, &TimelineSemaphore));
	}

	VulkanFence::~VulkanFence()
	{
		m_Context->GetFrameDeletionQueue().Push(TimelineSemaphore);
	}

	void VulkanFence::Wait()
	{
		GA_PROFILE_SCOPE();

		VkSemaphoreWaitInfo waitInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.semaphoreCount = 1,
			.pSemaphores = &TimelineSemaphore,
			.pValues = &Value
		};
		
		VK_CHECK(vkWaitSemaphores(m_Context->Device, &waitInfo, UINT64_MAX));
	}

}