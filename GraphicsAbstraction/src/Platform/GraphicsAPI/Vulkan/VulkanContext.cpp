#include "VulkanContext.h"

#include <VkBootstrap.h>
#include <GLFW/glfw3.h>

namespace GraphicsAbstraction {

	VulkanContext::VulkanContext()
	{
		vkb::InstanceBuilder builder;
		auto instRet = builder.set_app_name("Graphics Abstraction")
			.require_api_version(1, 1, 0)
#ifndef GA_DIST
			.request_validation_layers(true)
			.use_default_debug_messenger()
#endif
			.build();


		vkb::Instance vkbInstance = instRet.value();
		m_Instance = vkbInstance.instance;
		m_DebugMessenger = vkbInstance.debug_messenger;

		vkb::PhysicalDeviceSelector selector(vkbInstance);
		vkb::PhysicalDevice physicalDevice = selector
			.set_minimum_version(1, 1)
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
		m_DeletionQueue.Flush();

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

}