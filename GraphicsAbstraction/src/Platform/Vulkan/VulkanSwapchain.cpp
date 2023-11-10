#include "VulkanSwapchain.h"

#include <Platform/Vulkan/VulkanContext.h>
#include <VkBootstrap.h>

namespace GraphicsAbstraction {

	VulkanSwapchain::VulkanSwapchain(std::shared_ptr<Window> window, std::shared_ptr<GraphicsContext> context)
	{
		std::shared_ptr<VulkanContext> vulkanContext = std::dynamic_pointer_cast<VulkanContext>(context);
		m_Surface = vulkanContext->CreateSurface(window);

		vkb::SwapchainBuilder swapchainBuilder(vulkanContext->GetPhysicalDevice(), vulkanContext->GetLogicalDevice(), m_Surface);
		vkb::Swapchain vkbSwapchain = swapchainBuilder
			.use_default_format_selection()
			.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
			.set_desired_extent(window->GetWidth(), window->GetHeight())
			.build()
			.value();

		m_Swapchain = vkbSwapchain.swapchain;
		m_SwapchainImages = vkbSwapchain.get_images().value();
		m_SwapchainImageViews = vkbSwapchain.get_image_views().value();
		m_SwapchainImageFormat = vkbSwapchain.image_format;

		vulkanContext->PushToDeletionQueue([=]() {
			vkDestroySwapchainKHR(vulkanContext->GetLogicalDevice(), m_Swapchain, nullptr);
		});
	}

}