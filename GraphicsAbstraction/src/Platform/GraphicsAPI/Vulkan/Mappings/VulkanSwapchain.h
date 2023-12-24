#pragma once

#include <GraphicsAbstraction/Renderer/Swapchain.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanContext.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanImage.h>

#include <vulkan/vulkan.h>

namespace GraphicsAbstraction {

	class VulkanSurface;

	class VulkanSwapchain : public Swapchain
	{
	public:
		VkSwapchainKHR Swapchain;
		VkFormat ImageFormat;
		std::vector<std::shared_ptr<VulkanImage>> Images;

		std::vector<VkSemaphore> Semaphores;
		uint32_t SemaphoreIndex = 0;
		uint32_t ImageIndex = 0;

		uint32_t Width, Height;
		bool Dirty = false;
	public:
		VulkanSwapchain(const std::shared_ptr<Surface>& surface, const glm::vec2& size, bool enableVSync);
		virtual ~VulkanSwapchain();

		void Resize(uint32_t width, uint32_t height) override;
		void SetVsync(bool enabled) override;

		std::shared_ptr<Image> GetCurrent() override { return Images[ImageIndex]; }

		void Recreate();
	private:
		void CreateSwapchain(bool firstCreation);

		VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseVsyncOffPresent(const std::vector<VkPresentModeKHR>& presentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	private:
		VulkanContextReference m_Context;
		std::shared_ptr<VulkanSurface> m_Surface;
		bool m_EnableVsync;

		VkSurfaceFormatKHR m_ChosenSufaceFormat;
		VkPresentModeKHR m_VsyncOnPresent;
		VkPresentModeKHR m_VsyncOffPresent;
	};

}