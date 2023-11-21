#pragma once

#include <GraphicsAbstraction/Renderer/Swapchain.h>
#include <GraphicsAbstraction/Renderer/GraphicsContext.h>
#include <GraphicsAbstraction/Core/Window.h>

#include <Platform/GraphicsAPI/Vulkan/VulkanContext.h>
#include <Platform/GraphicsAPI/Vulkan/VulkanImage.h>

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
		virtual ~VulkanSwapchain();

		uint32_t AcquireNextImage() const override;
		void Resize(uint32_t width, uint32_t height) override;

		inline VkSurfaceKHR GetSurface() const { return m_Surface; }
		inline VkSwapchainKHR GetInternal() const { return m_Swapchain; }
		inline VkFormat GetImageFormat() const { return m_SwapchainImageFormat; }
		inline VkSemaphore GetPresentSemaphore() const { return m_PresentSemaphore; }
		inline VkSemaphore GetRenderSemaphore() const { return m_RenderSemaphore; }

		inline uint32_t GetWidth() const override { return (uint32_t)m_Size.x; }
		inline uint32_t GetHeight() const override { return (uint32_t)m_Size.y; }
		inline const glm::vec2& GetSize() const override { return m_Size; }

		inline const std::vector<std::shared_ptr<Image>>& GetImages() const override { return m_SwapchainImages; }

		inline uint32_t GetImageCount() const { return (uint32_t)m_SwapchainImages.size(); }
	private:
		void DestroySwapchain();

		void InitSwapchain();
		void InitSemaphores();
	private:
		VkSwapchainKHR m_Swapchain;
		VkSurfaceKHR m_Surface;
		VkFormat m_SwapchainImageFormat;
		VkSemaphore m_PresentSemaphore, m_RenderSemaphore;

		std::vector<std::shared_ptr<Image>> m_SwapchainImages;

		glm::vec2 m_Size;

		bool m_Initialized = false;
		std::shared_ptr<VulkanContext> m_Context;
	};
}