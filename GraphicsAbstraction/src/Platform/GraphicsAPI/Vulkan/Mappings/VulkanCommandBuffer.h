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

		void Clear(const Ref<Image>& image, const glm::vec4& color) override;
		void Present(const Ref<Swapchain>& swapchain) override;
		void Dispatch(uint32_t workX, uint32_t workY, uint32_t workZ) override;

		void CopyToBuffer(const Ref<Buffer>& src, const Ref<Buffer>& dst, uint32_t size, uint32_t srcOffset, uint32_t dstOffset) override;
		void CopyToImage(const Ref<Buffer>& src, const Ref<Image>& dst, uint32_t srcOffset) override;
		void CopyToImage(const Ref<Image>& src, const Ref<Image>& dst) override;
		void RWResourceBarrier(const Ref<Image>& resource) override;

		void BeginRendering(const glm::vec2& region, const std::vector<Ref<Image>>& colorAttachments, const Ref<Image>& depthAttachment) override;
		void EndRendering() override;

		void BindShaders(const std::vector<Ref<Shader>> shaderStages) override;
		void BindIndexBuffer(const Ref<Buffer>& buffer) override;
		void PushConstant(const void* data, uint32_t size, uint32_t offset) override;

		void SetViewport(const glm::vec2& size) override;
		void SetScissor(const glm::vec2& size, const glm::vec2& offset) override;
		void EnableDepthTest(bool writeEnabled, CompareOperation op) override;
		void DisableDepthTest() override;
		void EnableColorBlend(Blend srcBlend, Blend dstBlend, BlendOp blendOp, Blend srcBlendAlpha, Blend dstBlendAlpha, BlendOp blendAlpha) override;
		void DisableColorBlend() override;

		void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;
		void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) override;

		void DrawIndirect(const Ref<Buffer>& buffer, uint64_t offset, uint32_t drawCount, uint32_t stride) override;
		void DrawIndexedIndirect(const Ref<Buffer>& buffer, uint64_t offset, uint32_t drawCount, uint32_t stride) override;
	private:
		void SetColorBlend(bool enabled, Blend srcBlend, Blend dstBlend, BlendOp blendOp, Blend srcBlendAlpha, Blend dstBlendAlpha, BlendOp blendAlpha);
		void SetDynamicState();
	private:
		Ref<VulkanContext> m_Context;

		GraphicsPipelineKey m_GraphicsPipelineKey;
		ComputePipelineKey m_ComputePipelineKey;
		bool m_GraphicsPipelineStateChanged = false;
		bool m_ComputePipelineStateChanged = false;

		bool m_DefaultDynamicStateSet = false;
		bool m_DepthEnableSet = false;
		bool m_ColorBlendSet = false;
	};

}