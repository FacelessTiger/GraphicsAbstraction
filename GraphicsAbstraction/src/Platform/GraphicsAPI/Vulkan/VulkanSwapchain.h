#pragma once

#include <GraphicsAbstraction/Renderer/Swapchain.h>
#include <GraphicsAbstraction/Renderer/GraphicsContext.h>
#include <GraphicsAbstraction/Core/Window.h>

#include <vulkan/vulkan.h>
#include <vector>

namespace GraphicsAbstraction {

	class Window;
	class GraphicsContext;
	class VulkanContext;

	class VulkanSwapchain : public Swapchain
	{
	public:
		VulkanSwapchain(std::shared_ptr<Window> window, std::shared_ptr<GraphicsContext> context);
		virtual ~VulkanSwapchain() = default;

		uint32_t AcquireNextImage() const override;
		void SubmitCommandBuffer(std::shared_ptr<CommandBuffer> cmd, std::shared_ptr<Fence> fence) const override;
		void Present(uint32_t swapchainImageIndex) const override;

		inline VkSurfaceKHR GetSurface() const { return m_Surface; }
		inline VkFormat GetImageFormat() const { return m_SwapchainImageFormat; }

		inline uint32_t GetWidth() const { return m_Width; }
		inline uint32_t GetHeight() const { return m_Height; }

		inline uint32_t GetImageCount() const { return (uint32_t)m_SwapchainImages.size(); }
		inline const std::vector<VkImageView>& GetImageViews() const { return m_SwapchainImageViews; }
	private:
		void InitSwapchain(std::shared_ptr<Window> window);
		void InitSemaphores();
	private:
		VkSurfaceKHR m_Surface;
		VkSwapchainKHR m_Swapchain;
		VkFormat m_SwapchainImageFormat;
		VkSemaphore m_PresentSemaphore, m_RenderSemaphore;

		std::vector<VkImage> m_SwapchainImages;
		std::vector<VkImageView> m_SwapchainImageViews;

		uint32_t m_Width;
		uint32_t m_Height;

		std::shared_ptr<VulkanContext> m_Context;
	};
}