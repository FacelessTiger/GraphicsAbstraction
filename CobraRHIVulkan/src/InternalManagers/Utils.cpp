#include "Utils.h"

namespace GraphicsAbstraction::Utils {

	VkImageSubresourceRange ImageSubresourceRange(VkImageAspectFlags aspectMask)
	{
		VkImageSubresourceRange subImage{};
		subImage.aspectMask = aspectMask;
		subImage.baseMipLevel = 0;
		subImage.levelCount = 1;
		subImage.baseArrayLayer = 0;
		subImage.layerCount = 1;

		return subImage;
	}

	VkCompareOp GACompareOpToVulkan(CompareOperation op)
	{
		switch (op)
		{
			case CompareOperation::Never:			return VK_COMPARE_OP_NEVER;
			case CompareOperation::GreaterEqual:	return VK_COMPARE_OP_GREATER_OR_EQUAL;
			case CompareOperation::LesserEqual:		return VK_COMPARE_OP_LESS_OR_EQUAL;
		}

		assert(false && "Unknown compare operation!");
		return (VkCompareOp)0;
	}

	VkBlendFactor GABlendToVulkan(Blend blend)
	{
		switch (blend)
		{
			case Blend::Zero:				return VK_BLEND_FACTOR_ZERO;
			case Blend::One:				return VK_BLEND_FACTOR_ONE;
			case Blend::SrcAlpha:			return VK_BLEND_FACTOR_SRC_ALPHA;
			case Blend::DstAlpha:			return VK_BLEND_FACTOR_DST_ALPHA;
			case Blend::OneMinusSrcAlpha:	return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		}

		assert(false && "Unknown blend factor!");
		return (VkBlendFactor)0;
	}

	VkBlendOp GABlendOpToVulkan(BlendOp blendOp)
	{
		switch (blendOp)
		{
			case BlendOp::Add: return VK_BLEND_OP_ADD;
		}

		assert(false && "Unknown blend operation!");
		return (VkBlendOp)0;
	}

	VkFormat GAImageFormatToVulkan(ImageFormat format)
	{
		switch (format)
		{
			case ImageFormat::Unknown:				return VK_FORMAT_UNDEFINED;
			case ImageFormat::R16G16B16A16_SFLOAT:	return VK_FORMAT_R16G16B16A16_SFLOAT;
			case ImageFormat::R8G8B8A8_UNORM:		return VK_FORMAT_R8G8B8A8_UNORM;
			case ImageFormat::B8G8R8A8_SRGB:		return VK_FORMAT_B8G8R8A8_SRGB;
			case ImageFormat::D32_SFLOAT:			return VK_FORMAT_D32_SFLOAT;
		}

		assert(false && "Unknown image format!");
		return (VkFormat)0;
	}

	VkImageUsageFlags GAImageUsageToVulkan(ImageUsage usage)
	{
		VkImageUsageFlags ret = 0;

		if (usage & ImageUsage::ColorAttachment)		ret |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (usage & ImageUsage::DepthStencilAttachment)	ret |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		if (usage & ImageUsage::TransferSrc)			ret |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		if (usage & ImageUsage::TransferDst)			ret |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		if (usage & ImageUsage::Storage)				ret |= VK_IMAGE_USAGE_STORAGE_BIT;
		if (usage & ImageUsage::Sampled)				ret |= VK_IMAGE_USAGE_SAMPLED_BIT;

		return ret;
	}

	VkPolygonMode GAFillModeToVulkan(FillMode mode)
	{
		switch (mode)
		{
			case FillMode::Solid:		return VK_POLYGON_MODE_FILL;
			case FillMode::Wireframe:	return VK_POLYGON_MODE_LINE;
		}

		assert(false && "Unknown fill mode!");
		return (VkPolygonMode)0;
	}

	ImageFormat VulkanImageFormatToGA(VkFormat format)
	{
		switch (format)
		{
			case VK_FORMAT_R16G16B16A16_SFLOAT:	return ImageFormat::R16G16B16A16_SFLOAT;
			case VK_FORMAT_R8G8B8A8_UNORM:		return ImageFormat::R8G8B8A8_UNORM;
			case VK_FORMAT_B8G8R8A8_SRGB:		return ImageFormat::B8G8R8A8_SRGB;
			case VK_FORMAT_D32_SFLOAT:			return ImageFormat::D32_SFLOAT;
		}

		assert(false && "Unknown image format!");
		return ImageFormat::Unknown;
	}

	ImageUsage VulkanImageUsageToGA(VkImageUsageFlags usage)
	{
		ImageUsage ret = ImageUsage::None;

		if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)			ret |= ImageUsage::ColorAttachment;
		if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)	ret |= ImageUsage::DepthStencilAttachment;
		if (usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)				ret |= ImageUsage::TransferSrc;
		if (usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)				ret |= ImageUsage::TransferDst;
		if (usage & VK_IMAGE_USAGE_STORAGE_BIT)						ret |= ImageUsage::Storage;
		if (usage & VK_IMAGE_USAGE_SAMPLED_BIT)						ret |= ImageUsage::Sampled;

		return ret;
	}

}