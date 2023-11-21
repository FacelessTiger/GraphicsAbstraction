#include "VulkanImage.h"
#include "VulkanUtils.h"

#include <GraphicsAbstraction/Core/Core.h>
#include <Platform/GraphicsAPI/Vulkan/VulkanContext.h>

namespace GraphicsAbstraction {

	VulkanImage::VulkanImage(std::shared_ptr<GraphicsContext> context, const Specification& spec)
		: m_Layers(spec.Layers), m_Levels(spec.Levels)
	{
		m_Context = std::dynamic_pointer_cast<VulkanContext>(context);
		m_Samples = SampleNumberToVulkan(spec.Samples);
		m_Format = Utils::GAImageFormatToVulkan(spec.Format);

		VkImageUsageFlags usage = FormatToUsage(m_Format);
		VkImageAspectFlags aspect = UsageToAspect(usage);

		CreateImage(usage, { (uint32_t)spec.Size.x, (uint32_t)spec.Size.y, 1 });
		CreateImageView(aspect);
	}

	VulkanImage::VulkanImage(VkImage image, VkImageView imageView, VkFormat format)
		: m_Image(image), m_ImageView(imageView), m_Format(format), m_Samples(VK_SAMPLE_COUNT_1_BIT), m_HandledExternally(true)
	{ }

	VulkanImage::~VulkanImage()
	{
		if (!m_HandledExternally)
		{
			vkDestroyImageView(m_Context->GetLogicalDevice(), m_ImageView, nullptr);
			vmaDestroyImage(m_Context->GetAllocator(), m_Image, m_Allocation);
		}
	}

	VkImageUsageFlags VulkanImage::FormatToUsage(VkFormat format)
	{
		switch (format)
		{
			case VK_FORMAT_D32_SFLOAT:
				return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			case VK_FORMAT_B8G8R8A8_SRGB:
				return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}
	}

	VkSampleCountFlagBits VulkanImage::SampleNumberToVulkan(uint32_t samples)
	{
		// Round up to nearest power of 2 for a 32 bit int
		samples--;
		samples |= samples >> 1;
		samples |= samples >> 2;
		samples |= samples >> 4;
		samples |= samples >> 8;
		samples |= samples >> 16;
		samples++;

		switch (samples)
		{
			case 1:		return VK_SAMPLE_COUNT_1_BIT;
			case 2:		return VK_SAMPLE_COUNT_2_BIT;
			case 4:		return VK_SAMPLE_COUNT_4_BIT;
			case 8:		return VK_SAMPLE_COUNT_8_BIT;
			case 16:	return VK_SAMPLE_COUNT_16_BIT;
			case 32:	return VK_SAMPLE_COUNT_32_BIT;
			case 64:	return VK_SAMPLE_COUNT_64_BIT;
		}

		GA_CORE_ASSERT(false, "Number too high!");
		return VK_SAMPLE_COUNT_1_BIT;
	}

	VkImageAspectFlags VulkanImage::UsageToAspect(VkImageUsageFlags usage)
	{
		switch (usage)
		{
			case VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT:			return VK_IMAGE_ASPECT_COLOR_BIT;
			case VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT:	return VK_IMAGE_ASPECT_DEPTH_BIT;
		}

		GA_CORE_ASSERT(false, "Unknown image usgae!");
		return VK_IMAGE_ASPECT_COLOR_BIT;
	}

	void VulkanImage::CreateImage(VkImageUsageFlags usageFlags, VkExtent3D extent)
	{
		VkImageCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.pNext = nullptr;
		info.imageType = VK_IMAGE_TYPE_2D;
		info.format = m_Format;
		info.extent = extent;
		info.mipLevels = m_Levels;
		info.samples = m_Samples;
		info.arrayLayers = m_Layers;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = usageFlags;
		
		VmaAllocationCreateInfo vmaInfo = {};
		vmaInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		vmaInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VK_CHECK(vmaCreateImage(m_Context->GetAllocator(), &info, &vmaInfo, &m_Image, &m_Allocation, nullptr));
	}

	void VulkanImage::CreateImageView(VkImageAspectFlags aspect)
	{
		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.pNext = nullptr;
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.image = m_Image;
		info.format = m_Format;
		info.subresourceRange.baseMipLevel = 0;
		info.subresourceRange.levelCount = 1;
		info.subresourceRange.baseArrayLayer = 0;
		info.subresourceRange.layerCount = 1;
		info.subresourceRange.aspectMask = aspect;

		VK_CHECK(vkCreateImageView(m_Context->GetLogicalDevice(), &info, nullptr, &m_ImageView));
	}

}