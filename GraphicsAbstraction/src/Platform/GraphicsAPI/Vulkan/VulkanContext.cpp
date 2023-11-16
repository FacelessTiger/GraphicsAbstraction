#include "VulkanContext.h"

#include <VkBootstrap.h>
#include <GLFW/glfw3.h>

namespace GraphicsAbstraction {

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			switch (messageSeverity)
			{
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:	GA_CORE_WARN("Vulkan validation warn: {0}", pCallbackData->pMessage); return false;
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:		GA_CORE_ERROR("Vulkan validation error: {0}", pCallbackData->pMessage); return false;
			}
		}
	}

	VulkanContext::VulkanContext()
	{
		vkb::InstanceBuilder builder;
		auto instRet = builder.set_app_name("Graphics Abstraction")
			.require_api_version(1, 2, 0)
#ifndef GA_DIST
			.request_validation_layers(true)
			.set_debug_callback(debugCallback)
#endif
			.build();

		vkb::Instance vkbInstance = instRet.value();
		m_Instance = vkbInstance.instance;
		m_DebugMessenger = vkbInstance.debug_messenger;

		vkb::PhysicalDeviceSelector selector(vkbInstance);
		vkb::PhysicalDevice physicalDevice = selector
			.set_minimum_version(1, 2)
			.defer_surface_initialization()
			.select()
			.value();

		vkb::DeviceBuilder deviceBuilder(physicalDevice);
		vkb::Device vkbDevice = deviceBuilder.build().value();

		m_ChosenGPU = physicalDevice.physical_device;
		m_Device = vkbDevice.device;

		m_GraphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
		m_GraphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
	}

	VulkanContext::~VulkanContext()
	{
		vkDeviceWaitIdle(m_Device);
		m_DeletionQueue.Flush(*this);

		for (auto [surface, data] : m_SwapchainData)
		{
			vkDestroySwapchainKHR(m_Device, data.Swapchain, nullptr);
			for (VkImageView view : data.ImageViews)
				vkDestroyImageView(m_Device, view, nullptr);
		}

		for (auto [Renderpass, data] : m_FramebufferData)
		{
			for (VkFramebuffer framebuffer : data)
				vkDestroyFramebuffer(m_Device, framebuffer, nullptr);
		}

		vkDestroyDevice(m_Device, nullptr);

		for (auto [window, surface] : m_Surfaces)
			vkDestroySurfaceKHR(m_Instance, surface, nullptr);

#ifndef GA_DIST
		vkb::destroy_debug_utils_messenger(m_Instance, m_DebugMessenger);
#endif
		vkDestroyInstance(m_Instance, nullptr);
	}

	VkSurfaceKHR VulkanContext::CreateSurface(std::shared_ptr<Window> window)
	{
		if (m_Surfaces.contains(window)) return m_Surfaces.at(window);

		VkSurfaceKHR surface;
		VK_CHECK(glfwCreateWindowSurface(m_Instance, (GLFWwindow*)window->GetNativeWindow(), nullptr, &surface));

		m_Surfaces[window] = surface;
		return surface;
	}

	void VulkanContext::AddToSwapchainData(VkSurfaceKHR surface, const SwapchainData& data)
	{
		if (m_SwapchainData.contains(surface))
		{
			auto& oldData = m_SwapchainData[surface];

			vkDeviceWaitIdle(m_Device);
			vkDestroySwapchainKHR(m_Device, oldData.Swapchain, nullptr);
			for (VkImageView view : oldData.ImageViews)
				vkDestroyImageView(m_Device, view, nullptr);
		}

		m_SwapchainData[surface] = data;
	}

	void VulkanContext::AddToFramebufferData(VkRenderPass renderpass, const std::vector<VkFramebuffer>& framebuffers)
	{
		if (m_FramebufferData.contains(renderpass))
		{
			auto& oldData = m_FramebufferData[renderpass];

			vkDeviceWaitIdle(m_Device);
			for (VkFramebuffer framebuffer : oldData)
				vkDestroyFramebuffer(m_Device, framebuffer, nullptr);
		}

		m_FramebufferData[renderpass] = framebuffers;
	}

}