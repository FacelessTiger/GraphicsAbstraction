#include "VulkanUtils.h"

#include <Platform/GraphicsAPI/Vulkan/VulkanShader.h>

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

}