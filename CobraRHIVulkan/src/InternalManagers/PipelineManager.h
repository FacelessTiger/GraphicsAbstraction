#pragma once

#include <GraphicsAbstraction/Renderer/GraphicsContext.h>
#include <GraphicsAbstraction/Renderer/Shared/PipelineKeys.h>

#include <vulkan/vulkan.h>

namespace GraphicsAbstraction {

	class PipelineManager
	{
	public:
		PipelineManager(Impl<GraphicsContext>& context);
		~PipelineManager();

		VkPipeline GetGraphicsPipeline(const GraphicsPipelineKey& key);
		VkPipeline GetComputePipeline(const ComputePipelineKey& key);
	private:
		std::unordered_map<GraphicsPipelineKey, VkPipeline> m_GraphicsPipelines;
		std::unordered_map<ComputePipelineKey, VkPipeline> m_ComputePipelines;

		VkPipelineCache m_PipelineCache;
		Impl<GraphicsContext>& m_Context;
	};

}