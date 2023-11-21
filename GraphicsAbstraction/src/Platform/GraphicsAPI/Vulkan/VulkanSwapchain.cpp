#include "VulkanSwapchain.h"

#include <GraphicsAbstraction/Core/Core.h>
#include <GraphicsAbstraction/Debug/Instrumentor.h>

#include <VkBootstrap.h>
#include <GLFW/glfw3.h>

namespace GraphicsAbstraction {

	VulkanSwapchain::VulkanSwapchain(std::shared_ptr<Window> window, std::shared_ptr<GraphicsContext> context)
		: m_Size(window->GetWidth(), window->GetHeight())
	{
		GA_PROFILE_SCOPE();

		m_Context = std::dynamic_pointer_cast<VulkanContext>(context);
		VK_CHECK(glfwCreateWindowSurface(m_Context->GetInstance(), (GLFWwindow*)window->GetNativeWindow(), nullptr, &m_Surface));
		
		InitSwapchain();
		InitSemaphores();
	}

	VulkanSwapchain::~VulkanSwapchain()
	{
		DestroySwapchain();

		vkDestroySurfaceKHR(m_Context->GetInstance(), m_Surface, nullptr);
		vkDestroySemaphore(m_Context->GetLogicalDevice(), m_PresentSemaphore, nullptr);
		vkDestroySemaphore(m_Context->GetLogicalDevice(), m_RenderSemaphore, nullptr);
	}

	uint32_t VulkanSwapchain::AcquireNextImage() const
	{
		GA_PROFILE_SCOPE();

		uint32_t imageIndex;
		VK_CHECK(vkAcquireNextImageKHR(m_Context->GetLogicalDevice(), m_Swapchain, 1000000000, m_PresentSemaphore, nullptr, &imageIndex));

		return imageIndex;
	}

	void VulkanSwapchain::Resize(uint32_t width, uint32_t height)
	{
		m_Size.x = (float)width;
		m_Size.y = (float)height;

		InitSwapchain();
	}

	void VulkanSwapchain::DestroySwapchain()
	{
		vkDeviceWaitIdle(m_Context->GetLogicalDevice());

		for (auto& view : m_SwapchainImages)
			vkDestroyImageView(m_Context->GetLogicalDevice(), std::dynamic_pointer_cast<VulkanImage>(view)->GetView(), nullptr);

		vkDestroySwapchainKHR(m_Context->GetLogicalDevice(), m_Swapchain, nullptr);
	}

	void VulkanSwapchain::InitSwapchain()
	{
		GA_PROFILE_SCOPE();

		vkb::SwapchainBuilder swapchainBuilder(m_Context->GetPhysicalDevice(), m_Context->GetLogicalDevice(), m_Surface);

		swapchainBuilder
			.use_default_format_selection()
			.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
			.set_desired_extent((uint32_t)m_Size.x, (uint32_t)m_Size.y);

		if (m_Initialized) swapchainBuilder.set_old_swapchain(m_Swapchain);

		vkb::Swapchain vkbSwapchain = swapchainBuilder.build().value();

		if (m_Initialized) DestroySwapchain();

		m_Swapchain = vkbSwapchain.swapchain;

		auto images = vkbSwapchain.get_images().value();
		auto imageViews = vkbSwapchain.get_image_views().value();
		m_SwapchainImageFormat = vkbSwapchain.image_format;

		m_SwapchainImages.resize(images.size());
		for (int i = 0; i < images.size(); i++)
		{
			m_SwapchainImages[i] = std::make_shared<VulkanImage>(images[i], imageViews[i], m_SwapchainImageFormat);
		}

		m_Initialized = true;
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
	}

}