#include "VulkanBuffer.h"

#include <Platform/GraphicsAPI/Vulkan/VulkanContext.h>

namespace GraphicsAbstraction {

	namespace Utils {

		static VkBufferUsageFlags GABufferUsageToVulkan(BufferUsage usage)
		{
			VkBufferUsageFlags ret = 0;

			if (usage & BufferUsage::StorageBuffer)	ret |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			if (usage & BufferUsage::TransferSrc)	ret |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			if (usage & BufferUsage::TransferDst)	ret |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			if (usage & BufferUsage::IndexBuffer)	ret |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

			return ret;
		}

		static VmaAllocationCreateFlags GABufferFlagsToVMA(BufferFlags flags)
		{
			VmaAllocationCreateFlags ret = 0;

			// if its mapped, we're enforcing host access since no reason to map for gpu-only memory
			if (flags & BufferFlags::Mapped)			ret |= (VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT);
			if (flags & BufferFlags::DedicatedMemory)	ret |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

			return ret;
		}

	}

	VulkanBuffer::VulkanBuffer(uint32_t size, BufferUsage usage, BufferFlags flags)
		: m_Context(VulkanContext::GetReference()), Size(size)
	{
		VkBufferCreateInfo bufferInfo = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = (VkDeviceSize)Size,
			.usage = Utils::GABufferUsageToVulkan(usage),
		};

		VmaAllocationCreateInfo vmaAllocInfo = {
			.flags = Utils::GABufferFlagsToVMA(flags),
			.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
		};

		vmaCreateBuffer(m_Context->Allocator, &bufferInfo, &vmaAllocInfo, &Buffer.Buffer, &Buffer.Allocation, &Buffer.Info);
		if (usage & BufferUsage::StorageBuffer) UpdateDescriptor();
	}

	VulkanBuffer::~VulkanBuffer()
	{
		m_Context->GetFrameDeletionQueue().Push(Buffer);
	}

	void VulkanBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		uint32_t bufferSize = size ? size : Size; // If user passes a size use it, otherwise use creation size
		memcpy((char*)Buffer.Info.pMappedData + offset, data, bufferSize);
	}

	void VulkanBuffer::SetData(const std::shared_ptr<GraphicsAbstraction::Buffer>& buffer)
	{
		auto vulkanBuffer = std::static_pointer_cast<VulkanBuffer>(buffer);
		memcpy(Buffer.Info.pMappedData, vulkanBuffer->Buffer.Info.pMappedData, vulkanBuffer->Size);
	}

	void VulkanBuffer::GetData(void* data, uint32_t size, uint32_t offset)
	{
		memcpy(data, (char*)Buffer.Info.pMappedData + offset, size);
	}

	void VulkanBuffer::UpdateDescriptor()
	{
		VkDescriptorBufferInfo descriptorBufferInfo = {
			.buffer = Buffer.Buffer,
			.offset = 0,
			.range = VK_WHOLE_SIZE
		};

		VkWriteDescriptorSet write = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = m_Context->BindlessSet,
			.dstBinding = m_Context->STORAGE_BINDING,
			.dstArrayElement = Handle.GetValue(),
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.pBufferInfo = &descriptorBufferInfo
		};

		vkUpdateDescriptorSets(m_Context->Device, 1, &write, 0, nullptr);
	}

}