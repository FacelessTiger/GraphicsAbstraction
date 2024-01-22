#define VK_USE_PLATFORM_WIN32_KHR
#include <VulkanRHI.h>

#include <limits>
#include <string>
#include <algorithm>

#include <GraphicsAbstraction/Core/Window.h>

namespace GraphicsAbstraction {

	Ref<Swapchain> Swapchain::Create(const Ref<Window>& window, const glm::vec2& size, bool enableVSync)
	{
		auto swapchain = CreateRef<Swapchain>();
		swapchain->impl = new Impl<Swapchain>(window, size, enableVSync);
		return swapchain;
	}

	Swapchain::~Swapchain()
	{
		impl->Context->GetFrameDeletionQueue().Push(impl->Swapchain);
		impl->Context->GetFrameDeletionQueue().Push(impl->Surface);

		for (auto semaphore : impl->Semaphores)
			impl->Context->GetFrameDeletionQueue().Push(semaphore);
		delete impl;
	}

	void Swapchain::Resize(uint32_t width, uint32_t height)
	{
		impl->Width = width;
		impl->Height = height;

		for (auto& image : impl->Images)
		{
			image->impl->Width = width;
			image->impl->Height = height;
		}

		impl->Dirty = true;
		if (!g_DynamicRenderingSupported) impl->Context->RenderInfoManager->ClearFramebufferCache();
	}

	void Swapchain::SetVsync(bool enabled)
	{
		impl->EnableVsync = enabled;
		impl->Dirty = true;
	}

	Ref<Image> Swapchain::GetCurrent() 
	{ 
		return impl->Images[impl->ImageIndex]; 
	}

	Impl<Swapchain>::Impl(const Ref<Window>& window, const glm::vec2& size, bool enableVSync)
		: Context(Impl<GraphicsContext>::Reference), Width((uint32_t)size.x), Height((uint32_t)size.y), EnableVsync(enableVSync)
	{
		CreateSurface(window);
		CreateSwapchain(true);

		VkSemaphoreCreateInfo semaphoreInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = nullptr
		};

		Semaphores.resize(Images.size());
		for (int i = 0; i < Images.size(); i++)
		{
			VK_CHECK(vkCreateSemaphore(Context->Device, &semaphoreInfo, nullptr, &Semaphores[i]));
		}
	}

	void Impl<Swapchain>::Recreate()
	{
		CreateSwapchain(false);
		Dirty = false;
	}

	void Impl<Swapchain>::CreateSurface(const Ref<Window>& window)
	{
		VkWin32SurfaceCreateInfoKHR info = {
			.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			.hinstance = GetModuleHandle(nullptr),
			.hwnd = (HWND)window->GetNativeWindow()
		};
		VK_CHECK(vkCreateWin32SurfaceKHR(Context->Instance, &info, nullptr, &Surface));
	}

	void Impl<Swapchain>::CreateSwapchain(bool firstCreation)
	{
		if (firstCreation)
		{
			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(Context->ChosenGPU, Surface, &formatCount, nullptr);
			std::vector<VkSurfaceFormatKHR> formats(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(Context->ChosenGPU, Surface, &formatCount, formats.data());

			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(Context->ChosenGPU, Surface, &presentModeCount, nullptr);
			std::vector<VkPresentModeKHR> presentModes(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(Context->ChosenGPU, Surface, &presentModeCount, presentModes.data());

			ChosenSufaceFormat = ChooseSurfaceFormat(formats);
			VsyncOnPresent = VK_PRESENT_MODE_FIFO_KHR; // support for this is required
			VsyncOffPresent = ChooseVsyncOffPresent(presentModes);
		}

		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Context->ChosenGPU, Surface, &capabilities);

		VkExtent2D extent = ChooseSwapExtent(capabilities);
		Width = extent.width;
		Height = extent.height;

		uint32_t imageCount = capabilities.minImageCount + 1;
		imageCount = (capabilities.maxImageCount && imageCount > capabilities.maxImageCount) ? capabilities.maxImageCount : imageCount;

		VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		VkSwapchainCreateInfoKHR createInfo = {
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = Surface,
			.minImageCount = imageCount,
			.imageFormat = ChosenSufaceFormat.format,
			.imageColorSpace = ChosenSufaceFormat.colorSpace,
			.imageExtent = extent,
			.imageArrayLayers = 1,
			.imageUsage = usageFlags,
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.preTransform = capabilities.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = EnableVsync ? VsyncOnPresent : VsyncOffPresent,
			.clipped = VK_TRUE
		};

		VkSwapchainKHR oldSwapchain = Swapchain;
		if (!firstCreation) createInfo.oldSwapchain = oldSwapchain;
		VK_CHECK(vkCreateSwapchainKHR(Context->Device, &createInfo, nullptr, &Swapchain));
		if (!firstCreation) vkDestroySwapchainKHR(Context->Device, oldSwapchain, nullptr);

		vkGetSwapchainImagesKHR(Context->Device, Swapchain, &imageCount, nullptr);
		std::vector<VkImage> vulkanImages(imageCount);
		vkGetSwapchainImagesKHR(Context->Device, Swapchain, &imageCount, vulkanImages.data());

		ImageFormat = ChosenSufaceFormat.format;
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
			VK_CHECK(vkCreateImageView(Context->Device, &imageViewInfo, nullptr, &view));

			auto image = CreateRef<Image>();
			image->impl = new Impl<Image>(vulkanImages[i], view, VK_IMAGE_LAYOUT_UNDEFINED, Utils::VulkanImageFormatToGA(ImageFormat), Utils::VulkanImageUsageToGA(usageFlags), Width, Height);
			Images.push_back(image);
		}
	}

	VkSurfaceFormatKHR Impl<Swapchain>::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& format : availableFormats)
		{
			if (format.format == VK_FORMAT_R8G8B8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) return format;
		}

		return availableFormats[0];
	}

	VkPresentModeKHR Impl<Swapchain>::ChooseVsyncOffPresent(const std::vector<VkPresentModeKHR>& presentModes)
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

#undef max
	VkExtent2D Impl<Swapchain>::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) return capabilities.currentExtent;

		VkExtent2D actualExtent = { Width, Height };
		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}

}