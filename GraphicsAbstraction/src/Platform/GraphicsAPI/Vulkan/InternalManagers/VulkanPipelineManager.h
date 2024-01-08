#pragma once

#include <Platform/GraphicsAPI/Shared/PipelineKeys.h>

#include <vulkan/vulkan.h>

namespace GraphicsAbstraction {

	class VulkanContext;

	class VulkanPipelineManager
	{
	public:
		VulkanPipelineManager(VulkanContext& context);
		~VulkanPipelineManager();

		VkPipeline GetGraphicsPipeline(const GraphicsPipelineKey& key);
		VkPipeline GetComputePipeline(const ComputePipelineKey& key);
	private:
		std::unordered_map<GraphicsPipelineKey, VkPipeline> m_GraphicsPipelines;
		std::unordered_map<ComputePipelineKey, VkPipeline> m_ComputePipelines;

		VkPipelineCache m_PipelineCache;
		VulkanContext& m_Context;
	};

}