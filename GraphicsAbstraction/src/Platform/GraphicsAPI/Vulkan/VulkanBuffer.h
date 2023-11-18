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

		void Bind(std::shared_ptr<CommandBuffer> cmd) override;
	private:
		BufferLayout m_Layout;
		AllocatedBuffer m_VertexBuffer;

		std::shared_ptr<VulkanContext> m_Context;
	};

}