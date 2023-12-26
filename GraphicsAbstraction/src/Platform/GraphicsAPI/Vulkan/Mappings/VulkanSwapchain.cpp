#include "VulkanSwapchain.h"

#include <limits>
#include <algorithm>

#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanContext.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanSurface.h>

namespace GraphicsAbstraction {

	VulkanSwapchain::VulkanSwapchain(const std::shared_ptr<Surface>& surface, const glm::vec2& size, bool enableVSync)
		: m_Context(VulkanContext::GetReference()), m_Surface(std::static_pointer_cast<VulkanSurface>(surface)), Width((uint32_t)size.x), Height((uint32_t)size.y), m_EnableVsync(enableVSync)
	{
		CreateSwapchain(true);

		VkSemaphoreCreateInfo semaphoreInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = nullptr
		};

		Semaphores.resize(Images.size());
		for (int i = 0; i < Images.size(); i++)
		{
			VK_CHECK(vkCreateSemaphore(m_Context->Device, &semaphoreInfo, nullptr, &Semaphores[i]));
		}
	}

	VulkanSwapchain::~VulkanSwapchain()
	{
		m_Context->GetFrameDeletionQueue().Push(Swapchain);

		for (auto semaphore : Semaphores)
			m_Context->GetFrameDeletionQueue().Push(semaphore);
	}

	void VulkanSwapchain::Resize(uint32_t width, uint32_t height)
	{
		Width = width;
		Height = height;

		for (auto& image : Images)
		{
			image->Width = width;
			image->Height = height;
		}

		Dirty = true;
		if (!m_Context->DynamicRenderingSupported) m_Context->RenderInfoManager->ClearFramebufferCache();
	}

	void VulkanSwapchain::SetVsync(bool enabled)
	{
		m_EnableVsync = enabled;
		Dirty = true;
	}

	void VulkanSwapchain::Recreate()
	{
		CreateSwapchain(false);
		Dirty = false;
	}

	void VulkanSwapchain::CreateSwapchain(bool firstCreation)
	{
		if (firstCreation)
		{
			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(m_Context->ChosenGPU, m_Surface->Surface, &formatCount, nullptr);
			std::vector<VkSurfaceFormatKHR> formats(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(m_Context->ChosenGPU, m_Surface->Surface, &formatCount, formats.data());

			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(m_Context->ChosenGPU, m_Surface->Surface, &presentModeCount, nullptr);
			std::vector<VkPresentModeKHR> presentModes(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(m_Context->ChosenGPU, m_Surface->Surface, &presentModeCount, presentModes.data());

			m_ChosenSufaceFormat = ChooseSurfaceFormat(formats);
			m_VsyncOnPresent = VK_PRESENT_MODE_FIFO_KHR; // support for this is required
			m_VsyncOffPresent = ChooseVsyncOffPresent(presentModes);
		}

		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Context->ChosenGPU, m_Surface->Surface, &capabilities);

		VkExtent2D extent = ChooseSwapExtent(capabilities);
		Width = extent.width;
		Height = extent.height;

		uint32_t imageCount = capabilities.minImageCount + 1;
		imageCount = (capabilities.maxImageCount && imageCount > capabilities.maxImageCount) ? capabilities.maxImageCount : imageCount;

		VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		VkSwapchainCreateInfoKHR createInfo = {
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = m_Surface->Surface,
			.minImageCount = imageCount,
			.imageFormat = m_ChosenSufaceFormat.format,
			.imageColorSpace = m_ChosenSufaceFormat.colorSpace,
			.imageExtent = extent,
			.imageArrayLayers = 1,
			.imageUsage = usageFlags,
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.preTransform = capabilities.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = m_EnableVsync ? m_VsyncOnPresent : m_VsyncOffPresent,
			.clipped = VK_TRUE
		};

		VkSwapchainKHR oldSwapchain = Swapchain;
		if (!firstCreation) createInfo.oldSwapchain = oldSwapchain;
		VK_CHECK(vkCreateSwapchainKHR(m_Context->Device, &createInfo, nullptr, &Swapchain));
		if (!firstCreation) vkDestroySwapchainKHR(m_Context->Device, oldSwapchain, nullptr);

		vkGetSwapchainImagesKHR(m_Context->Device, Swapchain, &imageCount, nullptr);
		std::vector<VkImage> vulkanImages(imageCount);
		vkGetSwapchainImagesKHR(m_Context->Device, Swapchain, &imageCount, vulkanImages.data());

		ImageFormat = m_ChosenSufaceFormat.format;
		Images.clear();

		for (uint32_t i = 0; i < vulkanImages.size(); i++)
		{
			VkImageSubresourceRange range = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			};
			VkImageViewCreateInfo imageViewInfo = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = vulkanImages[i],
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = ImageFormat,
				.subresourceRange = range
			};

			VkImageView view;
			VK_CHECK(vkCreateImageView(m_Context->Device, &imageViewInfo, nullptr, &view));

			Images.push_back(std::make_shared<VulkanImage>(vulkanImages[i], view, VK_IMAGE_LAYOUT_UNDEFINED, ImageFormat, usageFlags, Width, Height));
		}
	}

	VkSurfaceFormatKHR VulkanSwapchain::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& format : availableFormats)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) return format;
		}

		return availableFormats[0];
	}

	VkPresentModeKHR VulkanSwapchain::ChooseVsyncOffPresent(const std::vector<VkPresentModeKHR>& presentModes)
	{
		// return immediately if mailbox, otherwise prefer immediate and if not available default to fifo
		VkPresentModeKHR ret = VK_PRESENT_MODE_FIFO_KHR;
		for (const auto& present : presentModes)
		{
			if (present == VK_PRESENT_MODE_MAILBOX_KHR) return present;
			if (present == VK_PRESENT_MODE_IMMEDIATE_KHR) ret = present;
		}

		return ret;
	}

	VkExtent2D VulkanSwapchain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) return capabilities.currentExtent;

		VkExtent2D actualExtent = { Width, Height };
		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}

}