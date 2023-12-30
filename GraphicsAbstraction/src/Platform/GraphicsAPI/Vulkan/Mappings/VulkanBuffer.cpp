#include "VulkanBuffer.h"

#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanContext.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanImage.h>

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

	}

	Ref<Buffer> Buffer::Create(uint32_t size, BufferUsage usage, BufferFlags flags)
	{
		return CreateRef<VulkanBuffer>(size, usage, flags);
	}

	VulkanBuffer::VulkanBuffer(uint32_t size, BufferUsage usage, BufferFlags flags)
		: m_Context(VulkanContext::GetReference()), Size(size)
	{
		VmaAllocationCreateFlags vmaFlags = {};
		if (flags & BufferFlags::Mapped)
		{
			vmaFlags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
			if (flags & BufferFlags::DeviceLocal) vmaFlags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
			else vmaFlags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
		}

		VkBufferCreateInfo bufferInfo = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = (VkDeviceSize)Size,
			.usage = Utils::GABufferUsageToVulkan(usage),
		};

		VmaAllocationCreateInfo vmaAllocInfo = {
			.flags = vmaFlags,
			.usage = VMA_MEMORY_USAGE_AUTO,
			.requiredFlags = (flags & BufferFlags::DeviceLocal) ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : (VkMemoryPropertyFlags)0
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

	void VulkanBuffer::SetData(const Ref<GraphicsAbstraction::Buffer>& buffer)
	{
		auto& vulkanBuffer = (VulkanBuffer&)*buffer;
		memcpy(Buffer.Info.pMappedData, vulkanBuffer.Buffer.Info.pMappedData, vulkanBuffer.Size);
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