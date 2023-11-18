#include "VulkanContext.h"

#include <VkBootstrap.h>

#include <GraphicsAbstraction/Debug/Instrumentor.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

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

		return false;
	}

	VulkanContext::VulkanContext()
	{
		GA_PROFILE_SCOPE();

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
#ifndef GA_DIST
			.add_required_extension("VK_EXT_calibrated_timestamps")
#endif
			.select()
			.value();

		vkb::DeviceBuilder deviceBuilder(physicalDevice);
		vkb::Device vkbDevice = deviceBuilder.build().value();
		
		m_ChosenGPU = physicalDevice.physical_device;
		m_Device = vkbDevice.device;

		m_GraphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
		m_GraphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

#ifndef GA_DIST
		m_GetPhysicalDeviceCalibrateableTimeDomainsEXT = (PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT)vkGetDeviceProcAddr(m_Device, "vkGetPhysicalDeviceCalibrateableTimeDomainsEXT");
		m_GetCalibratedTimestampsEXT = (PFN_vkGetCalibratedTimestampsEXT)vkGetDeviceProcAddr(m_Device, "vkGetCalibratedTimestampsEXT");
#endif

		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = m_ChosenGPU;
		allocatorInfo.device = m_Device;
		allocatorInfo.instance = m_Instance;
		vmaCreateAllocator(&allocatorInfo, &m_Allocator);
	}

	VulkanContext::~VulkanContext()
	{
		GA_PROFILE_SCOPE();

		vkDeviceWaitIdle(m_Device);
		vmaDestroyAllocator(m_Allocator);
		vkDestroyDevice(m_Device, nullptr);

#ifndef GA_DIST
		vkb::destroy_debug_utils_messenger(m_Instance, m_DebugMessenger);
#endif
		vkDestroyInstance(m_Instance, nullptr);
	}

}