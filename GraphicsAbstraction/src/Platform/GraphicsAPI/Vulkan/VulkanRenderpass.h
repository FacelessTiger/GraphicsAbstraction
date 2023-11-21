#pragma once

#include <GraphicsAbstraction/Renderer/Renderpass.h>
#include <GraphicsAbstraction/Renderer/Swapchain.h>

#include <vulkan/vulkan.h>

namespace GraphicsAbstraction {

	class VulkanContext;
	class VulkanSwapchain;

	class VulkanRenderpass : public Renderpass
	{
	public:
		VulkanRenderpass(std::shared_ptr<GraphicsContext> context, const Specification& spec);
		virtual ~VulkanRenderpass();

		void Begin(const glm::vec2& size, std::shared_ptr<CommandBuffer> cmd, const std::vector<ClearValue>& clearValues, uint32_t swapchainImageIndex) const override;
		void End(std::shared_ptr<CommandBuffer> cmd) const override;

		void Recreate(const Specification& spec) override;

		inline VkRenderPass GetInternal() const { return m_Renderpass; }
		inline VkPipelineBindPoint GetBindpoint() const { return m_Bindpoint; }
	private:
		void DestroyFramebuffers();

		void InitRenderpass(const Specification& spec);
		void CreateFramebuffers(const Specification& spec);
	private:
		VkRenderPass m_Renderpass;
		std::vector<VkFramebuffer> m_Framebuffers;

		VkPipelineBindPoint m_Bindpoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // hard coding bindpoint to graphics for now

		std::shared_ptr<VulkanContext> m_Context;
	};

}