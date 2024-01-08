#pragma once

#include <memory>
#include <vector>

#include <GraphicsAbstraction/Core/Core.h>
#include <GraphicsAbstraction/Renderer/Image.h>
#include <glm/glm.hpp>

namespace GraphicsAbstraction {

	class Swapchain;
	class Image;
	class Buffer;
	class Shader;

	enum class CompareOperation
	{
		Never = 0,
		GreaterEqual,
		LesserEqual
	};

	enum class Blend
	{
		Zero,
		One,
		SrcAlpha,
		DstAlpha,
		OneMinusSrcAlpha
	};

	enum class BlendOp
	{
		Add
	};

	class CommandBuffer : public RefCounted
	{
	public:
		virtual ~CommandBuffer() = default;

		virtual void Clear(const Ref<Image>& image, const glm::vec4& color) = 0;
		virtual void Present(const Ref<Swapchain>& swapchain) = 0;
		virtual void Dispatch(uint32_t workX, uint32_t workY, uint32_t workZ) = 0;

		virtual void CopyToBuffer(const Ref<Buffer>& src, const Ref<Buffer>& dst, uint32_t size, uint32_t srcOffset = 0, uint32_t dstOffset = 0) = 0;
		virtual void CopyToImage(const Ref<Buffer>& src, const Ref<Image>& dst, uint32_t srcOffset = 0) = 0;
		virtual void CopyToImage(const Ref<Image>& src, const Ref<Image>& dst) = 0;
		virtual void RWResourceBarrier(const Ref<Image>& resource) = 0;

		virtual void BeginRendering(const glm::vec2& region, const std::vector<Ref<Image>>& colorAttachments, const Ref<Image>& depthAttachment = nullptr) = 0;
		virtual void EndRendering() = 0;

		virtual void BindShaders(const std::vector<Ref<Shader>> shaderStages) = 0;
		virtual void BindIndexBuffer(const Ref<Buffer>& buffer) = 0;
		virtual void PushConstant(const void* data, uint32_t size, uint32_t offset) = 0;
		template<typename T> void PushConstant(const T& data, uint32_t offset = 0) { PushConstant(&data, sizeof(T), offset); };

		virtual void SetViewport(const glm::vec2& size) = 0;
		virtual void SetScissor(const glm::vec2& size, const glm::vec2& offset = { 0, 0 }) = 0;
		virtual void SetDepthTest(bool testEnabled, bool writeEnabled, CompareOperation op) = 0;
		virtual void EnableColorBlend(Blend srcBlend, Blend dstBlend, BlendOp blendOp, Blend srcBlendAlpha, Blend dstBlendAlpha, BlendOp blendAlpha) = 0;
		virtual void DisableColorBlend() = 0;

		virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;
		virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) = 0;
	};

}