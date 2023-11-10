#include "VulkanContext.h"

#include <VkBootstrap.h>
#include <GLFW/glfw3.h>

namespace GraphicsAbstraction {

	VulkanContext::VulkanContext()
	{
		vkb::InstanceBuilder builder;
		auto instRet = builder.set_app_name("Graphics Abstraction")
			.request_validation_layers(m_EnableValidation)
			.require_api_version(1, 1, 0)
			.use_default_debug_messenger()
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
	}

	VulkanContext::~VulkanContext()
	{
		m_DeletionQueue.Flush();

		vkDestroyDevice(m_Device, nullptr);

		for (int i = 0; i < m_Surfaces.size(); i++)
			vkDestroySurfaceKHR(m_Instance, m_Surfaces[i], nullptr);

		vkb::destroy_debug_utils_messenger(m_Instance, m_DebugMessenger);
		vkDestroyInstance(m_Instance, nullptr);
	}

	VkSurfaceKHR VulkanContext::CreateSurface(std::shared_ptr<Window> window)
	{
		VkSurfaceKHR surface;
		VK_CHECK(glfwCreateWindowSurface(m_Instance, (GLFWwindow*)window->GetNativeWindow(), nullptr, &surface));

		m_Surfaces.push_back(surface);
		return surface;
	}

}