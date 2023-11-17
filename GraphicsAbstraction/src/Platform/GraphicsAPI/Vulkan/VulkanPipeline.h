#pragma once

#include <GraphicsAbstraction/Renderer/Pipeline.h>

#include <vulkan/vulkan.h>

namespace GraphicsAbstraction {

	class VulkanPipeline : public Pipeline
	{
	public:
		VulkanPipeline(std::shared_ptr<GraphicsContext> context, std::shared_ptr<Shader> shader, std::shared_ptr<Renderpass> renderpass, uint32_t width, uint32_t height);
		virtual ~VulkanPipeline() = default;

		void Bind(std::shared_ptr<CommandBuffer> cmd, Renderpass::PipelineBindpoint bindpoint) const override;
	private:
		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_Pipeline;
	};

}