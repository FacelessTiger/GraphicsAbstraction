#include "VulkanBuffer.h"

#include <GraphicsAbstraction/Debug/Instrumentor.h>

namespace GraphicsAbstraction {

	VulkanVertexBuffer::VulkanVertexBuffer(std::shared_ptr<GraphicsContext> context, uint32_t size)
		: m_Context(std::dynamic_pointer_cast<VulkanContext>(context))
	{
		GA_PROFILE_SCOPE();

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

		VmaAllocationCreateInfo vmaAllocInfo = {};
		vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		VK_CHECK(vmaCreateBuffer(m_Context->GetAllocator(), &bufferInfo, &vmaAllocInfo, &m_VertexBuffer.Buffer, &m_VertexBuffer.Allocation, nullptr));
	}

	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		vmaDestroyBuffer(m_Context->GetAllocator(), m_VertexBuffer.Buffer, m_VertexBuffer.Allocation);
	}

	void VulkanVertexBuffer::SetData(const void* data, uint32_t size)
	{
		GA_PROFILE_SCOPE();
		void* gpuData;

		vmaMapMemory(m_Context->GetAllocator(), m_VertexBuffer.Allocation, &gpuData);
		memcpy(gpuData, data, size);
		vmaUnmapMemory(m_Context->GetAllocator(), m_VertexBuffer.Allocation);
	}

	void VulkanVertexBuffer::Bind(std::shared_ptr<CommandBuffer> cmd) const
	{
		GA_PROFILE_SCOPE();
		std::shared_ptr<VulkanCommandBuffer> vulkanCommandBuffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(cmd);

		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(vulkanCommandBuffer->GetInternal(), 0, 1, &m_VertexBuffer.Buffer, &offset);
	}

	VulkanIndexBuffer::VulkanIndexBuffer(std::shared_ptr<GraphicsContext> context, uint32_t* indices, uint32_t count)
		: m_Context(std::dynamic_pointer_cast<VulkanContext>(context)), m_Count(count)
	{
		GA_PROFILE_SCOPE();
		size_t size = count * sizeof(uint32_t);

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

		VmaAllocationCreateInfo vmaAllocInfo = {};
		vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		VK_CHECK(vmaCreateBuffer(m_Context->GetAllocator(), &bufferInfo, &vmaAllocInfo, &m_IndexBuffer.Buffer, &m_IndexBuffer.Allocation, nullptr));

		void* gpuData;
		vmaMapMemory(m_Context->GetAllocator(), m_IndexBuffer.Allocation, &gpuData);
		memcpy(gpuData, indices, size);
		vmaUnmapMemory(m_Context->GetAllocator(), m_IndexBuffer.Allocation);
	}

	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
		vmaDestroyBuffer(m_Context->GetAllocator(), m_IndexBuffer.Buffer, m_IndexBuffer.Allocation);
	}

	void VulkanIndexBuffer::Bind(std::shared_ptr<CommandBuffer> cmd) const
	{
		GA_PROFILE_SCOPE();
		std::shared_ptr<VulkanCommandBuffer> vulkanCommandBuffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(cmd);

		vkCmdBindIndexBuffer(vulkanCommandBuffer->GetInternal(), m_IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);
	}

}