#pragma once

#include <GraphicsAbstraction/Renderer/Swapchain.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanContext.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanImage.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanSurface.h>

#include <vulkan/vulkan.h>

namespace GraphicsAbstraction {

	class VulkanSwapchain : public Swapchain
	{
	public:
		VkSwapchainKHR Swapchain;
		VkFormat ImageFormat;
		std::vector<Ref<VulkanImage>> Images;

		std::vector<VkSemaphore> Semaphores;
		uint32_t SemaphoreIndex = 0;
		uint32_t ImageIndex = 0;

		uint32_t Width, Height;
		bool Dirty = false;
	public:
		VulkanSwapchain(Ref<Surface>& surface, const glm::vec2& size, bool enableVSync);
		virtual ~VulkanSwapchain();

		void Resize(uint32_t width, uint32_t height) override;
		void SetVsync(bool enabled) override;

		Ref<Image> GetCurrent() override { return Images[ImageIndex]; }

		void Recreate();
	private:
		void CreateSwapchain(bool firstCreation);

		VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseVsyncOffPresent(const std::vector<VkPresentModeKHR>& presentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	private:
		VulkanContextReference m_Context;
		Ref<VulkanSurface> m_Surface;
		bool m_EnableVsync;

		VkSurfaceFormatKHR m_ChosenSufaceFormat;
		VkPresentModeKHR m_VsyncOnPresent;
		VkPresentModeKHR m_VsyncOffPresent;
	};

}