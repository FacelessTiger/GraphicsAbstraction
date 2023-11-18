#pragma once

#include <GraphicsAbstraction/Renderer/Pipeline.h>

#include <vulkan/vulkan.h>

namespace GraphicsAbstraction {

	class VulkanContext;

	class VulkanPipeline : public Pipeline
	{
	public:
		VulkanPipeline(std::shared_ptr<GraphicsContext> context, const Specification& spec);
		virtual ~VulkanPipeline();

		void Bind(std::shared_ptr<CommandBuffer> cmd, Renderpass::PipelineBindpoint bindpoint) const override;
	private:
		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_Pipeline;

		std::shared_ptr<VulkanContext> m_Context;
	};

}