#pragma once

#include <vulkan/vulkan.h>
#include <xxhash.h>

#include <vector>
#include <array>
#include <unordered_map>

#define GA_PIPELINE_EQUALITY(name) bool operator==(const name& other) const = default
#define GA_PIPELINE_HASH(name)															\
	template<>																			\
	struct hash<GraphicsAbstraction::name>												\
	{																					\
		std::size_t operator()(const GraphicsAbstraction::name& key) const				\
		{																				\
			return (std::size_t)XXH64(&key, sizeof(GraphicsAbstraction::name), 0);		\
		}																				\
	}	

namespace GraphicsAbstraction {

	struct VulkanGraphicsPipelineKey
	{
		// attachments/shaders
		std::array<uint32_t, 5> Shaders = {};
		std::array<VkFormat, 8> ColorAttachments = {};
		VkFormat DepthAttachment = VK_FORMAT_UNDEFINED;

		// depth test
		bool DepthTestEnable = false;
		bool DepthWriteEnable = false;
		VkCompareOp DepthCompareOp = VK_COMPARE_OP_NEVER;

		// color blending
		bool BlendEnable = false;
		VkBlendFactor SrcBlend = VK_BLEND_FACTOR_ZERO;
		VkBlendFactor DstBlend = VK_BLEND_FACTOR_ZERO;
		VkBlendFactor SrcBlendAlpha = VK_BLEND_FACTOR_ZERO;
		VkBlendFactor DstBlendAlpha = VK_BLEND_FACTOR_ZERO;
		VkBlendOp BlendOp = VK_BLEND_OP_ADD;
		VkBlendOp BlendOpAlpha = VK_BLEND_OP_ADD;

		VkRenderPass Renderpass = 0;

		GA_PIPELINE_EQUALITY(VulkanGraphicsPipelineKey);
	};

	struct VulkanComputePipelineKey
	{
		uint32_t Shader;

		GA_PIPELINE_EQUALITY(VulkanComputePipelineKey);
	};

}

namespace std {

	GA_PIPELINE_HASH(VulkanGraphicsPipelineKey);
	GA_PIPELINE_HASH(VulkanComputePipelineKey);

}

#undef GA_PIPELINE_EQUALITY
#undef GA_PIPELINE_HASH