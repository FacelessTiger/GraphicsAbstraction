#pragma once

#include <GraphicsAbstraction/Renderer/CommandBuffer.h>
#include <GraphicsAbstraction/Renderer/Shader.h>
#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Context.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace GraphicsAbstraction {

	class D3D12CommandBuffer : public CommandBuffer
	{
	public:
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList;
	public:
		D3D12CommandBuffer(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);

		void Clear(const Ref<Image>& image, const glm::vec4& color) override;
		void Present(const Ref<Swapchain>& swapchain) override;
		void Dispatch(uint32_t workX, uint32_t workY, uint32_t workZ) override;

		void CopyToBuffer(const Ref<Buffer>& src, const Ref<Buffer>& dst, uint32_t size, uint32_t srcOffset = 0, uint32_t dstOffset = 0) override;
		void CopyToImage(const Ref<Buffer>& src, const Ref<Image>& dst, uint32_t srcOffset = 0) override;
		void CopyToImage(const Ref<Image>& src, const Ref<Image>& dst) override;
		void RWResourceBarrier(const Ref<Image>& resource) override;

		void BeginRendering(const glm::vec2& region, const std::vector<Ref<Image>>& colorAttachments, const Ref<Image>& depthAttachment = nullptr) override;
		void EndRendering() override { }

		void BindShaders(const std::vector<Ref<Shader>> shaderStages) override;
		void BindIndexBuffer(const Ref<Buffer>& buffer) override;
		void PushConstant(const void* data, uint32_t size, uint32_t offset) override;

		void SetViewport(const glm::vec2& size) override;
		void SetScissor(const glm::vec2& size, const glm::vec2& offset = { 0, 0 }) override;
		void SetDepthTest(bool testEnabled, bool writeEnabled, CompareOperation op) override;
		void EnableColorBlend(Blend srcBlend, Blend dstBlend, BlendOp blendOp, Blend srcBlendAlpha, Blend dstBlendAlpha, BlendOp blendAlpha) override;
		void DisableColorBlend() override;

		void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;
		void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) override;
	private:
		void SetGraphicsPipeline();
	private:
		Ref<D3D12Context> m_Context;

		GraphicsPipelineKey m_GraphicsPipelineKey;
		ComputePipelineKey m_ComputePipelineKey;
		bool m_GraphicsPipelineStateChanged = false;
		bool m_ComputePipelineStateChanged = false;

		bool m_DefaultDynamicStateSet = false;
	};

}