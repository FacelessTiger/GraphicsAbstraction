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
		VulkanRenderpass(const Specification& spec, std::shared_ptr<GraphicsContext> context, std::shared_ptr<Swapchain> swapchain);
		virtual ~VulkanRenderpass();

		void Begin(std::shared_ptr<Swapchain> swapchain, std::shared_ptr<CommandBuffer> cmd, const glm::vec4& clearColor, uint32_t swapchainImageIndex) const override;
		void End(std::shared_ptr<CommandBuffer> cmd) const override;

		void Recreate(std::shared_ptr<Swapchain> swapchain) override;

		inline VkRenderPass GetInternal() const { return m_Renderpass; }
	private:
		void DestroyFramebuffers();

		void InitRenderpass(const Specification& spec, std::shared_ptr<VulkanSwapchain> swapchain);
		void CreateFramebuffers(std::shared_ptr<Swapchain> swapchain);
	private:
		VkRenderPass m_Renderpass;
		std::vector<VkFramebuffer> m_Framebuffers;

		std::shared_ptr<VulkanContext> m_Context;
	};

}