#pragma once

#include <GraphicsAbstraction/Renderer/GraphicsContext.h>
#include <GraphicsAbstraction/Core/Window.h>

#include <Platform/GraphicsAPI/Vulkan/VulkanDeletionQueue.h>

#include <vulkan/vulkan.h>

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

	class VulkanSwapchain;

	struct SwapchainData
	{
		VkSwapchainKHR Swapchain;
		std::vector<VkImageView> ImageViews;
	};

	class VulkanContext : public GraphicsContext
	{
	public:
		VulkanContext();
		virtual ~VulkanContext();

		VkSurfaceKHR CreateSurface(std::shared_ptr<Window> window);
		void AddToSwapchainData(VkSurfaceKHR surface, const SwapchainData& data);
		void AddToFramebufferData(VkRenderPass renderpass, const std::vector<VkFramebuffer>& framebuffers);
		
		inline void PushToDeletionQueue(std::function<void(VulkanContext&)>&& function) { m_DeletionQueue.PushFunction(std::move(function)); }

		inline VkInstance GetInstance() const { return m_Instance; }
		inline VkPhysicalDevice GetPhysicalDevice() const { return m_ChosenGPU; }
		inline VkDevice GetLogicalDevice() const { return m_Device; }

		inline VkQueue GetGraphicsQeue() const { return m_GraphicsQueue; }
		inline uint32_t GetGraphicsQueueFamily() const { return m_GraphicsQueueFamily; }

		inline PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT GetPhysicalDeviceCalibrateableTimeDomainsEXT() { return m_GetPhysicalDeviceCalibrateableTimeDomainsEXT; }
		inline PFN_vkGetCalibratedTimestampsEXT GetCalibratedTimestampsEXT() { return m_GetCalibratedTimestampsEXT; }
	private:
		VkInstance m_Instance;
		VkDebugUtilsMessengerEXT m_DebugMessenger;
		VkPhysicalDevice m_ChosenGPU;
		VkDevice m_Device;

		std::unordered_map<VkSurfaceKHR, SwapchainData> m_SwapchainData;
		std::unordered_map<VkRenderPass, std::vector<VkFramebuffer>> m_FramebufferData;
		std::unordered_map<std::shared_ptr<Window>, VkSurfaceKHR> m_Surfaces;
		VulkanDeletionQueue m_DeletionQueue;

		VkQueue m_GraphicsQueue;
		uint32_t m_GraphicsQueueFamily;

		PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT m_GetPhysicalDeviceCalibrateableTimeDomainsEXT;
		PFN_vkGetCalibratedTimestampsEXT m_GetCalibratedTimestampsEXT;
	};

}