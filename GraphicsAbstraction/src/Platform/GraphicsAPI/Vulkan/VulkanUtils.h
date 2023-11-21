#pragma once

#include <GraphicsAbstraction/Renderer/Image.h>
#include <GraphicsAbstraction/Debug/Instrumentor.h>

#include <vulkan/vulkan.h>

namespace GraphicsAbstraction { namespace Utils {

	static VkFormat GAImageFormatToVulkan(Image::Format format)
	{
		switch (format)
		{
		case Image::Format::BGRA8SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
		case Image::Format::D32SFloat: return VK_FORMAT_D32_SFLOAT;
		}

		GA_CORE_ASSERT(false, "Unknown image format!");
		return VK_FORMAT_B8G8R8A8_SRGB;
	}

} }