#pragma once

#include <Platform/GraphicsAPI/Vulkan/InternalManagers/VulkanPipelineKeys.h>

#include <vulkan/vulkan.h>

namespace GraphicsAbstraction {

	class VulkanContext;

	class VulkanPipelineManager
	{
	public:
		VulkanPipelineManager(VulkanContext& context);
		~VulkanPipelineManager();

		VkPipeline GetGraphicsPipeline(const VulkanGraphicsPipelineKey& key);
		VkPipeline GetComputePipeline(const VulkanComputePipelineKey& key);
	private:
		std::unordered_map<VulkanGraphicsPipelineKey, VkPipeline> m_GraphicsPipelines;
		std::unordered_map<VulkanComputePipelineKey, VkPipeline> m_ComputePipelines;

		VkPipelineCache m_PipelineCache;
		VulkanContext& m_Context;
	};

}