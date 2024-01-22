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

	struct RenderInfoAttachment
	{
		ImageFormat Format = ImageFormat::Unknown;
		VkImageLayout InitialLayout;
		ImageUsage Usage = ImageUsage::None;

		GA_RENDERINFO_EQUALITY(RenderInfoAttachment);
	};

	struct RenderInfoKey
	{
		std::array<RenderInfoAttachment, 8> ColorAttachments = {};
		RenderInfoAttachment DepthAttachment;

		uint32_t Width;
		uint32_t Height;

		GA_RENDERINFO_EQUALITY(RenderInfoKey);
	};

}

namespace std {

	GA_RENDERINFO_HASH(RenderInfoKey);

}

#undef GA_RENDERPASS_EQUALITY
#undef GA_RENDERPASS_HASH