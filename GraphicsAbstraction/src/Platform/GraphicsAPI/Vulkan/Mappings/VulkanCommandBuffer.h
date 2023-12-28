#pragma once

#include <GraphicsAbstraction/Renderer/CommandBuffer.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanContext.h>

#include <vulkan/vulkan.h>

namespace GraphicsAbstraction {

	class VulkanCommandBuffer : public CommandBuffer
	{
	public:
		VkCommandBuffer CommandBuffer;
	public:
		VulkanCommandBuffer(VkCommandBuffer buffer);
		virtual ~VulkanCommandBuffer();

		void Clear(const std::shared_ptr<Image>& image, const glm::vec4& color) override;
		void Present(const std::shared_ptr<Swapchain>& swapchain) override;
		void Dispatch(uint32_t workX, uint32_t workY, uint32_t workZ) override;
		void CopyToBuffer(const std::shared_ptr<Buffer>& src, const std::shared_ptr<Buffer>& dst, uint32_t size, uint32_t srcOffset, uint32_t dstOffset) override;
		void CopyToImage(const std::shared_ptr<Buffer>& src, const std::shared_ptr<Image>& dst, uint32_t srcOffset) override;
		void BeginRendering(const glm::vec2& region, const std::vector<std::shared_ptr<Image>>& colorAttachments, const std::shared_ptr<Image>& depthAttachment) override;
		void EndRendering() override;

		void BindShaders(const std::vector<std::shared_ptr<Shader>> shaderStages) override;
		void BindIndexBuffer(const std::shared_ptr<Buffer>& buffer) override;
		void PushConstant(const void* data, uint32_t size, uint32_t offset) override;

		void SetViewport(const glm::vec2& size) override;
		void SetScissor(const glm::vec2& size, const glm::vec2& offset) override;
		void SetDepthTest(bool testEnabled, bool writeEnabled, CompareOperation op) override;
		void EnableColorBlend(Blend srcBlend, Blend dstBlend, BlendOp blendOp, Blend srcBlendAlpha, Blend dstBlendAlpha, BlendOp blendAlpha) override;
		void DisableColorBlend() override;

		void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;
		void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) override;
	private:
		void SetColorBlend(bool enabled, VkBlendFactor srcBlend, VkBlendFactor dstBlend, VkBlendOp blendOp, VkBlendFactor srcBlendAlpha, VkBlendFactor dstBlendAlpha, VkBlendOp blendAlpha);
		void SetDynamicState();
	private:
		VulkanContextReference m_Context;

		VulkanGraphicsPipelineKey m_GraphicsPipelineKey;
		VulkanComputePipelineKey m_ComputePipelineKey;
		bool m_GraphicsPipelineStateChanged = false;
		bool m_ComputePipelineStateChanged = false;

		bool m_DefaultDynamicStateSet = false;
		bool m_DepthEnableSet = false;
		bool m_ColorBlendSet = false;
	};

}