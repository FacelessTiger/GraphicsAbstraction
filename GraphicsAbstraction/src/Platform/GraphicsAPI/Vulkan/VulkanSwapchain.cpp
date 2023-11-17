#include "VulkanSwapchain.h"

#include <Platform/GraphicsAPI/Vulkan/VulkanContext.h>
#include <GraphicsAbstraction/Core/Core.h>
#include <GraphicsAbstraction/Debug/Instrumentor.h>

#include <VkBootstrap.h>

namespace GraphicsAbstraction {

	VulkanSwapchain::VulkanSwapchain(std::shared_ptr<Window> window, std::shared_ptr<GraphicsContext> context)
		: m_Width(window->GetWidth()), m_Height(window->GetHeight())
	{
		GA_PROFILE_SCOPE();
		m_Context = std::dynamic_pointer_cast<VulkanContext>(context);
		
		InitSwapchain(window);
		InitSemaphores();
	}

	uint32_t VulkanSwapchain::AcquireNextImage() const
	{
		GA_PROFILE_SCOPE();

		uint32_t imageIndex;
		VK_CHECK(vkAcquireNextImageKHR(m_Context->GetLogicalDevice(), m_SwapchainData.Swapchain, 1000000000, m_PresentSemaphore, nullptr, &imageIndex));

		return imageIndex;
	}

	void VulkanSwapchain::Resize(uint32_t width, uint32_t height)
	{
		m_Width = width;
		m_Height = height;

		CreateSwapchain();
	}

	void VulkanSwapchain::InitSwapchain(std::shared_ptr<Window> window)
	{
		m_Surface = m_Context->CreateSurface(window);

		CreateSwapchain();
	}

	void VulkanSwapchain::InitSemaphores()
	{
		GA_PROFILE_SCOPE();

		VkSemaphoreCreateInfo semaphoreCreateInfo = {};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCreateInfo.pNext = nullptr;
		semaphoreCreateInfo.flags = 0;

		VK_CHECK(vkCreateSemaphore(m_Context->GetLogicalDevice(), &semaphoreCreateInfo, nullptr, &m_PresentSemaphore));
		VK_CHECK(vkCreateSemaphore(m_Context->GetLogicalDevice(), &semaphoreCreateInfo, nullptr, &m_RenderSemaphore));

		m_Context->PushToDeletionQueue([presentSemaphore = m_PresentSemaphore, renderSemaphore = m_RenderSemaphore](VulkanContext& context) {
			vkDestroySemaphore(context.GetLogicalDevice(), presentSemaphore, nullptr);
			vkDestroySemaphore(context.GetLogicalDevice(), renderSemaphore, nullptr);
		});
	}

	void VulkanSwapchain::CreateSwapchain()
	{
		GA_PROFILE_SCOPE();

		vkb::SwapchainBuilder swapchainBuilder(m_Context->GetPhysicalDevice(), m_Context->GetLogicalDevice(), m_Surface);

		swapchainBuilder
			.use_default_format_selection()
			.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
			.set_desired_extent(m_Width, m_Height);

		if (m_Initialized)
			swapchainBuilder.set_old_swapchain(m_SwapchainData.Swapchain);

		vkb::Swapchain vkbSwapchain = swapchainBuilder.build().value();

		m_SwapchainData.Swapchain = vkbSwapchain.swapchain;
		m_SwapchainImages = vkbSwapchain.get_images().value();
		m_SwapchainData.ImageViews = vkbSwapchain.get_image_views().value();
		m_SwapchainImageFormat = vkbSwapchain.image_format;

		m_Context->AddToSwapchainData(m_Surface, m_SwapchainData);
		m_Initialized = true;
	}

}