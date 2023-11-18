#include "VulkanFence.h"

#include <Platform/GraphicsAPI/Vulkan/VulkanContext.h>

#include <GraphicsAbstraction/Debug/Instrumentor.h>

namespace GraphicsAbstraction {

	VulkanFence::VulkanFence(std::shared_ptr<GraphicsContext> context)
	{
		GA_PROFILE_SCOPE();

		m_Context = std::dynamic_pointer_cast<VulkanContext>(context);

		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.pNext = nullptr;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VK_CHECK(vkCreateFence(m_Context->GetLogicalDevice(), &fenceCreateInfo, nullptr, &m_Fence));
	}

	VulkanFence::~VulkanFence()
	{
		vkDestroyFence(m_Context->GetLogicalDevice(), m_Fence, nullptr);
	}

	void VulkanFence::Wait() const
	{
		GA_PROFILE_SCOPE();

		VK_CHECK(vkWaitForFences(m_Context->GetLogicalDevice(), 1, &m_Fence, true, 1000000000));
		VK_CHECK(vkResetFences(m_Context->GetLogicalDevice(), 1, &m_Fence));
	}

}