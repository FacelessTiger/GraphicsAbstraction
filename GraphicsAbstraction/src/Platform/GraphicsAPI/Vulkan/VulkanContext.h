#pragma once

#include <GraphicsAbstraction/Renderer/GraphicsContext.h>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <map>

struct GLFWwindow;

#define VK_CHECK(x)										\
	do													\
	{													\
		VkResult err = x;								\
		if (err)										\
		{												\
			GA_CORE_ERROR("Vulkan error: {0}", err);	\
			GA_CORE_ASSERT(false);						\
		}												\
	} while (0);

namespace GraphicsAbstraction {

	class VulkanContext : public GraphicsContext
	{
	public:
		VulkanContext();
		virtual ~VulkanContext();

		inline VkInstance GetInstance() const { return m_Instance; }
		inline VkPhysicalDevice GetPhysicalDevice() const { return m_ChosenGPU; }
		inline VkDevice GetLogicalDevice() const { return m_Device; }

		inline VmaAllocator GetAllocator() const { return m_Allocator; }

		inline VkQueue GetGraphicsQeue() const { return m_GraphicsQueue; }
		inline uint32_t GetGraphicsQueueFamily() const { return m_GraphicsQueueFamily; }

		inline PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT GetPhysicalDeviceCalibrateableTimeDomainsEXT() { return m_GetPhysicalDeviceCalibrateableTimeDomainsEXT; }
		inline PFN_vkGetCalibratedTimestampsEXT GetCalibratedTimestampsEXT() { return m_GetCalibratedTimestampsEXT; }
	private:
		VkInstance m_Instance;
		VkDebugUtilsMessengerEXT m_DebugMessenger;
		VkPhysicalDevice m_ChosenGPU;
		VkDevice m_Device;

		VmaAllocator m_Allocator;

		VkQueue m_GraphicsQueue;
		uint32_t m_GraphicsQueueFamily;

		PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT m_GetPhysicalDeviceCalibrateableTimeDomainsEXT;
		PFN_vkGetCalibratedTimestampsEXT m_GetCalibratedTimestampsEXT;
	};

}