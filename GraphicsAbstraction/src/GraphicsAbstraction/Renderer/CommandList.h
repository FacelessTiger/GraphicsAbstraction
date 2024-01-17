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

	enum class FillMode
	{
		Solid,
		Wireframe
	};

	struct DrawIndirectCommand
	{
	public:
		DrawIndirectCommand(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
			: m_VertexCount(vertexCount), m_InstanceCount(instanceCount), m_FirstVertex(firstVertex), m_FirstInstance(firstInstance), m_Reserved(firstVertex), m_Reserved2(firstInstance)
		{ }
	private:
		uint32_t m_Reserved, m_Reserved2;
		uint32_t m_VertexCount;
		uint32_t m_InstanceCount;
		uint32_t m_FirstVertex;
		uint32_t m_FirstInstance;
	};

	struct DrawIndexedIndirectCommand
	{
	public:
		DrawIndexedIndirectCommand(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
			: m_IndexCount(indexCount), m_InstanceCount(instanceCount), m_FirstIndex(firstIndex), m_VertexOffset(vertexOffset), m_FirstInstance(firstInstance), m_Reserved(vertexOffset), m_Reserved2(firstInstance)
		{ }
	private:
		uint32_t m_Reserved, m_Reserved2;
		uint32_t m_IndexCount;
		uint32_t m_InstanceCount;
		uint32_t m_FirstIndex;
		uint32_t m_VertexOffset;
		uint32_t m_FirstInstance;
	};

	struct DispatchIndirectCommand
	{
		uint32_t workX, workY, workZ;
	};

	class CommandList : public RefCounted
	{
	public:
		virtual ~CommandList() = default;

		virtual void Clear(const Ref<Image>& image, const glm::vec4& color) = 0;
		virtual void Present(const Ref<Swapchain>& swapchain) = 0;

		virtual void CopyBufferRegion(const Ref<Buffer>& src, const Ref<Buffer>& dst, uint32_t size, uint32_t srcOffset = 0, uint32_t dstOffset = 0) = 0;
		virtual void CopyToImage(const Ref<Buffer>& src, const Ref<Image>& dst, uint32_t srcOffset = 0) = 0;
		virtual void CopyToImage(const Ref<Image>& src, const Ref<Image>& dst) = 0;
		virtual void RWResourceBarrier(const Ref<Image>& resource) = 0;
		virtual void RWResourceBarrier(const Ref<Buffer>& resource) = 0;

		virtual void BeginRendering(const glm::vec2& region, const std::vector<Ref<Image>>& colorAttachments, const Ref<Image>& depthAttachment = nullptr) = 0;
		virtual void EndRendering() = 0;

		virtual void BindShaders(const std::vector<Ref<Shader>> shaderStages) = 0;
		virtual void BindIndexBuffer(const Ref<Buffer>& buffer) = 0;
		virtual void PushConstant(const void* data, uint32_t size, uint32_t offset) = 0;
		template<typename T> void PushConstant(const T& data, uint32_t offset = 0) { PushConstant(&data, sizeof(T), offset); };

		virtual void SetViewport(const glm::vec2& size) = 0;
		virtual void SetScissor(const glm::vec2& size, const glm::vec2& offset = { 0, 0 }) = 0;
		virtual void SetFillMode(FillMode mode) = 0;
		virtual void EnableDepthTest(bool writeEnabled, CompareOperation op) = 0;
		virtual void DisableDepthTest() = 0;
		virtual void EnableColorBlend(Blend srcBlend, Blend dstBlend, BlendOp blendOp, Blend srcBlendAlpha, Blend dstBlendAlpha, BlendOp blendAlpha) = 0;
		virtual void DisableColorBlend() = 0;

		virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;
		virtual void DrawIndirect(const Ref<Buffer>& buffer, uint64_t offset, uint32_t drawCount, uint32_t stride) = 0;
		virtual void DrawIndirectCount(const Ref<Buffer>& buffer, uint64_t offset, const Ref<Buffer>& countBuffer, uint64_t countOffset, uint32_t maxDrawCount, uint32_t stride) = 0;

		virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) = 0;
		virtual void DrawIndexedIndirect(const Ref<Buffer>& buffer, uint64_t offset, uint32_t drawCount, uint32_t stride) = 0;
		virtual void DrawIndexedIndirectCount(const Ref<Buffer>& buffer, uint64_t offset, const Ref<Buffer>& countBuffer, uint64_t countOffset, uint32_t maxDrawCount, uint32_t stride) = 0;

		virtual void Dispatch(uint32_t workX, uint32_t workY, uint32_t workZ) = 0;
		virtual void DispatchIndirect(const Ref<Buffer>& buffer, uint64_t offset) = 0;
	};

}