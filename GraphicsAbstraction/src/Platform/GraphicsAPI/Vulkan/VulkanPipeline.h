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

		inline VkPipelineLayout GetLayout() const { return m_PipelineLayout; }

		void Bind(std::shared_ptr<CommandBuffer> cmd) const override;
	private:
		VkPipelineLayout m_PipelineLayout;
		VkPipelineBindPoint m_Bindpoint;
		VkPipeline m_Pipeline;

		std::shared_ptr<VulkanContext> m_Context;
	};

}