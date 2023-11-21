#pragma once

#include <GraphicsAbstraction/Renderer/Buffer.h>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace GraphicsAbstraction {

	class VulkanContext;

	struct AllocatedBuffer
	{
		VkBuffer Buffer;
		VmaAllocation Allocation;
	};

	class VulkanVertexBuffer : public VertexBuffer
	{
	public:
		VulkanVertexBuffer(std::shared_ptr<GraphicsContext> context, uint32_t size);
		virtual ~VulkanVertexBuffer();

		void SetData(const void* data, uint32_t size) override;

		void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
		const BufferLayout& GetLayout() const override { return m_Layout; }

		void Bind(std::shared_ptr<CommandBuffer> cmd) const override;
	private:
		BufferLayout m_Layout;
		AllocatedBuffer m_VertexBuffer;

		std::shared_ptr<VulkanContext> m_Context;
	};

	class VulkanIndexBuffer : public IndexBuffer
	{
	public:
		VulkanIndexBuffer(std::shared_ptr<GraphicsContext> context, uint32_t* indices, uint32_t count);
		virtual ~VulkanIndexBuffer();

		void Bind(std::shared_ptr<CommandBuffer> cmd) const override;

		inline uint32_t GetCount() const { return m_Count; }
	private:
		AllocatedBuffer m_IndexBuffer;
		uint32_t m_Count;

		std::shared_ptr<VulkanContext> m_Context;
	};

}