#include "VulkanBuffer.h"

namespace GraphicsAbstraction {

	VulkanVertexBuffer::VulkanVertexBuffer(std::shared_ptr<GraphicsContext> context, uint32_t size)
		: m_Context(std::dynamic_pointer_cast<VulkanContext>(context))
	{
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
		void* gpuData;

		vmaMapMemory(m_Context->GetAllocator(), m_VertexBuffer.Allocation, &gpuData);
		memcpy(gpuData, data, size);
		vmaUnmapMemory(m_Context->GetAllocator(), m_VertexBuffer.Allocation);
	}

	void VulkanVertexBuffer::Bind(std::shared_ptr<CommandBuffer> cmd)
	{
		std::shared_ptr<VulkanCommandBuffer> vulkanCommandBuffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(cmd);

		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(vulkanCommandBuffer->GetInternal(), 0, 1, &m_VertexBuffer.Buffer, &offset);
	}

}