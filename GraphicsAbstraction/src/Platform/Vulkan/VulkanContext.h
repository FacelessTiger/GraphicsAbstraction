#pragma once

#include <GraphicsAbstraction/Renderer/GraphicsContext.h>
#include <GraphicsAbstraction/Core/Window.h>

#include <Platform/Vulkan/VulkanSwapchain.h>
#include <Platform/Vulkan/VulkanDeletionQueue.h>

#include <vulkan/vulkan.h>

#include <vector>
#include <iostream>

struct GLFWwindow;

#define VK_CHECK(x)																\
	do																			\
	{																			\
		VkResult err = x;														\
		if (err)																\
		{																		\
			std::cerr << "Detected Vulkan error: " << err << std::endl;			\
			abort();															\
		}																		\
	} while (0);

namespace GraphicsAbstraction {

	class VulkanContext : public GraphicsContext
	{
	public:
		VulkanContext();
		virtual ~VulkanContext();

		VkSurfaceKHR CreateSurface(std::shared_ptr<Window> window);
		
		inline void PushToDeletionQueue(std::function<void()>&& function) { m_DeletionQueue.PushFunction(std::move(function)); }

		inline VkInstance GetInstance() const { return m_Instance; }
		inline VkPhysicalDevice GetPhysicalDevice() const { return m_ChosenGPU; }
		inline VkDevice GetLogicalDevice() const { return m_Device; }
	private:
#ifndef GA_DIST
		static constexpr bool m_EnableValidation = true;
#else
		static constexpr bool m_EnableValidation = false;
#endif
		VkInstance m_Instance;
		VkDebugUtilsMessengerEXT m_DebugMessenger;
		VkPhysicalDevice m_ChosenGPU;
		VkDevice m_Device;

		std::vector<VkSurfaceKHR> m_Surfaces;
		VulkanDeletionQueue m_DeletionQueue;
	};

}