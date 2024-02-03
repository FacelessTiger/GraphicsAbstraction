#pragma once

#include <memory>
#include <vector>

#include <GraphicsAbstraction/Core/Core.h>
#include <GraphicsAbstraction/Renderer/Image.h>
#include <glm/glm.hpp>

namespace GraphicsAbstraction {

	struct Swapchain;
	struct Image;
	struct Buffer;
	struct Shader;

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

	enum class IndexType
	{
		Uint16,
		Uint32
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

	struct GA_DLL_LINK CommandList : public RefCounted
	{
		Impl<CommandList>* impl;
		virtual ~CommandList();

		void Clear(const Ref<Image>& image, const glm::vec4& color);
		void Present(const Ref<Swapchain>& swapchain);
		
		void CopyBufferRegion(const Ref<Buffer>& src, const Ref<Buffer>& dst, uint32_t size, uint32_t srcOffset = 0, uint32_t dstOffset = 0);
		void CopyToImage(const Ref<Buffer>& src, const Ref<Image>& dst, uint32_t srcOffset = 0);
		void CopyToImage(const Ref<Image>& src, const Ref<Image>& dst);
		void RWResourceBarrier(const Ref<Image>& resource);
		void RWResourceBarrier(const Ref<Buffer>& resource);

		void BeginRendering(const glm::vec2& region, const std::vector<Ref<Image>>& colorAttachments, const Ref<Image>& depthAttachment = nullptr);
		void EndRendering();

		void BindShaders(const std::vector<Ref<Shader>> shaderStages);
		void BindIndexBuffer(const Ref<Buffer>& buffer, IndexType type);
		void PushConstant(const void* data, uint32_t size, uint32_t offset);
		template<typename T> void PushConstant(const T& data, uint32_t offset = 0) { PushConstant(&data, sizeof(T), offset); };

		void SetViewport(const glm::vec2& size);
		void SetScissor(const glm::vec2& size, const glm::vec2& offset = { 0, 0 });
		void SetFillMode(FillMode mode);
		void EnableDepthTest(bool writeEnabled, CompareOperation op);
		void DisableDepthTest();
		void EnableColorBlend(Blend srcBlend, Blend dstBlend, BlendOp blendOp, Blend srcBlendAlpha, Blend dstBlendAlpha, BlendOp blendAlpha);
		void DisableColorBlend();

		void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
		void DrawIndirect(const Ref<Buffer>& buffer, uint64_t offset, uint32_t drawCount, uint32_t stride);
		void DrawIndirectCount(const Ref<Buffer>& buffer, uint64_t offset, const Ref<Buffer>& countBuffer, uint64_t countOffset, uint32_t maxDrawCount, uint32_t stride);

		void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance);
		void DrawIndexedIndirect(const Ref<Buffer>& buffer, uint64_t offset, uint32_t drawCount, uint32_t stride);
		void DrawIndexedIndirectCount(const Ref<Buffer>& buffer, uint64_t offset, const Ref<Buffer>& countBuffer, uint64_t countOffset, uint32_t maxDrawCount, uint32_t stride);

		void Dispatch(uint32_t workX, uint32_t workY, uint32_t workZ);
		void DispatchIndirect(const Ref<Buffer>& buffer, uint64_t offset);
	};

}