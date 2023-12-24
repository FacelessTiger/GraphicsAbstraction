#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>

namespace GraphicsAbstraction {

	struct VulkanGraphicsPipelineKey
	{
		std::vector<uint32_t> Shaders;
		std::vector<VkFormat> ColorAttachments;
		VkFormat DepthAttachment = VK_FORMAT_UNDEFINED;

		bool operator==(const VulkanGraphicsPipelineKey& other) const = default;
	};

	struct VulkanComputePipelineKey
	{
		uint32_t Shader;

		bool operator==(const VulkanComputePipelineKey& other) const = default;
	};

}

namespace std {

	template<>
	struct hash<GraphicsAbstraction::VulkanGraphicsPipelineKey>
	{
		std::size_t operator()(const GraphicsAbstraction::VulkanGraphicsPipelineKey& key) const
		{
			std::size_t ret = 0;

			for (uint32_t shaderID : key.Shaders) ret ^= std::hash<uint32_t>()(shaderID);
			for (VkFormat format : key.ColorAttachments) ret ^= std::hash<uint32_t>()((uint32_t)format);
			ret ^= std::hash<uint32_t>()((uint32_t)key.DepthAttachment);

			return ret;
		}
	};

	template<>
	struct hash<GraphicsAbstraction::VulkanComputePipelineKey>
	{
		std::size_t operator()(const GraphicsAbstraction::VulkanComputePipelineKey& key) const
		{
			return std::hash<uint32_t>()(key.Shader);
		}
	};

}