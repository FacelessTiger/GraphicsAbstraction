#pragma once

#include <vulkan/vulkan.h>
#include <xxhash.h>

#include <vector>
#include <array>
#include <unordered_map>

#include <GraphicsAbstraction/Renderer/Image.h>

#define GA_RENDERINFO_EQUALITY(name) bool operator==(const name& other) const = default
#define GA_RENDERINFO_HASH(name)														\
	template<>																			\
	struct hash<GraphicsAbstraction::name>												\
	{																					\
		std::size_t operator()(const GraphicsAbstraction::name& key) const				\
		{																				\
			return (std::size_t)XXH64(&key, sizeof(GraphicsAbstraction::name), 0);		\
		}																				\
	}	

namespace GraphicsAbstraction {

	struct VulkanRenderInfoAttachment
	{
		ImageFormat Format = ImageFormat::Unknown;
		VkImageLayout InitialLayout;
		ImageUsage Usage = ImageUsage::None;

		GA_RENDERINFO_EQUALITY(VulkanRenderInfoAttachment);
	};

	struct VulkanRenderInfoKey
	{
		std::array<VulkanRenderInfoAttachment, 8> ColorAttachments = {};
		VulkanRenderInfoAttachment DepthAttachment;

		uint32_t Width;
		uint32_t Height;

		GA_RENDERINFO_EQUALITY(VulkanRenderInfoKey);
	};

}

namespace std {

	GA_RENDERINFO_HASH(VulkanRenderInfoKey);

}

#undef GA_RENDERPASS_EQUALITY
#undef GA_RENDERPASS_HASH