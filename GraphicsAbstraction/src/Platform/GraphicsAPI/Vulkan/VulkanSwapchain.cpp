#include "VulkanSwapchain.h"

#include <Platform/GraphicsAPI/Vulkan/VulkanContext.h>
#include <Platform/GraphicsAPI/Vulkan/VulkanCommandBuffer.h>
#include <Platform/GraphicsAPI/Vulkan/VulkanFence.h>

#include <VkBootstrap.h>

namespace GraphicsAbstraction {

	VulkanSwapchain::VulkanSwapchain(std::shared_ptr<Window> window, std::shared_ptr<GraphicsContext> context)
		: m_Width(window->GetWidth()), m_Height(window->GetHeight())
	{
		m_Context = std::dynamic_pointer_cast<VulkanContext>(context);
		
		InitSwapchain(window);
		InitSemaphores();
	}

	uint32_t VulkanSwapchain::AcquireNextImage() const
	{
		uint32_t imageIndex;
		VK_CHECK(vkAcquireNextImageKHR(m_Context->GetLogicalDevice(), m_Swapchain, 1000000000, m_PresentSemaphore, nullptr, &imageIndex));

		return imageIndex;
	}

	void VulkanSwapchain::SubmitCommandBuffer(std::shared_ptr<CommandBuffer> cmd, std::shared_ptr<Fence> fence) const
	{
		std::shared_ptr<VulkanCommandBuffer> vulkanCommandBuffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(cmd);
		std::shared_ptr<VulkanFence> vulkanFence = std::dynamic_pointer_cast<VulkanFence>(fence);
		VkCommandBuffer vkCmd = vulkanCommandBuffer->GetInternal();

		VkSubmitInfo submit = {};
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit.pNext = nullptr;

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submit.pWaitDstStageMask = &waitStage;

		submit.waitSemaphoreCount = 1;
		submit.pWaitSemaphores = &m_PresentSemaphore;
		submit.signalSemaphoreCount = 1;
		submit.pSignalSemaphores = &m_RenderSemaphore;

		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &vkCmd;

		VK_CHECK(vkQueueSubmit(m_Context->GetGraphicsQeue(), 1, &submit, vulkanFence->GetInternal()));
	}

	void VulkanSwapchain::Present(uint32_t swapchainImageIndex) const
	{
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;

		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_Swapchain;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &m_RenderSemaphore;
		presentInfo.pImageIndices = &swapchainImageIndex;
		
		VK_CHECK(vkQueuePresentKHR(m_Context->GetGraphicsQeue(), &presentInfo));
	}

	void VulkanSwapchain::InitSwapchain(std::shared_ptr<Window> window)
	{
		m_Surface = m_Context->CreateSurface(window);

		vkb::SwapchainBuilder swapchainBuilder(m_Context->GetPhysicalDevice(), m_Context->GetLogicalDevice(), m_Surface);
		vkb::Swapchain vkbSwapchain = swapchainBuilder
			.use_default_format_selection()
			.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
			.set_desired_extent(m_Width, m_Height)
			.build()
			.value();

		m_Swapchain = vkbSwapchain.swapchain;
		m_SwapchainImages = vkbSwapchain.get_images().value();
		m_SwapchainImageViews = vkbSwapchain.get_image_views().value();
		m_SwapchainImageFormat = vkbSwapchain.image_format;

		m_Context->PushToDeletionQueue([=]() {
			vkDestroySwapchainKHR(m_Context->GetLogicalDevice(), m_Swapchain, nullptr);
		});

		for (int i = 0; i < m_SwapchainImageViews.size(); i++)
		{
			m_Context->PushToDeletionQueue([=]() {
				vkDestroyImageView(m_Context->GetLogicalDevice(), m_SwapchainImageViews[i], nullptr);
			});
		}
	}

	void VulkanSwapchain::InitSemaphores()
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo = {};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCreateInfo.pNext = nullptr;
		semaphoreCreateInfo.flags = 0;

		VK_CHECK(vkCreateSemaphore(m_Context->GetLogicalDevice(), &semaphoreCreateInfo, nullptr, &m_PresentSemaphore));
		VK_CHECK(vkCreateSemaphore(m_Context->GetLogicalDevice(), &semaphoreCreateInfo, nullptr, &m_RenderSemaphore));
	}

}