#pragma once

#include <GraphicsAbstraction/Renderer/Swapchain.h>
#include <GraphicsAbstraction/Renderer/GraphicsContext.h>
#include <GraphicsAbstraction/Core/Window.h>

#include <vulkan/vulkan.h>
#include <vector>

namespace GraphicsAbstraction {

	class Window;
	class GraphicsContext;

	class VulkanSwapchain : public Swapchain
	{
	public:
		VulkanSwapchain(std::shared_ptr<Window> window, std::shared_ptr<GraphicsContext> context);
		virtual ~VulkanSwapchain() = default;

		inline VkSurfaceKHR GetSurface() const { return m_Surface; }
	private:
		VkSurfaceKHR m_Surface;
		VkSwapchainKHR m_Swapchain;
		VkFormat m_SwapchainImageFormat;

		std::vector<VkImage> m_SwapchainImages;
		std::vector<VkImageView> m_SwapchainImageViews;
	};
}