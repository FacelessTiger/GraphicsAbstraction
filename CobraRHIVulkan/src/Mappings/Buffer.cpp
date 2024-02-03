#include <VulkanRHI.h>

namespace GraphicsAbstraction {

	namespace Utils {

		static VkBufferUsageFlags GABufferUsageToVulkan(BufferUsage usage)
		{
			VkBufferUsageFlags ret = 0;

			if (usage & BufferUsage::StorageBuffer)		ret |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			if (usage & BufferUsage::TransferSrc)		ret |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			if (usage & BufferUsage::TransferDst)		ret |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			if (usage & BufferUsage::IndexBuffer)		ret |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			if (usage & BufferUsage::IndirectBuffer)	ret |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

			return ret;
		}

	}

	Ref<Buffer> Buffer::Create(uint32_t size, BufferUsage usage, BufferFlags flags)
	{
		auto buffer = CreateRef<Buffer>();
		buffer->impl = new Impl<Buffer>(size, usage, flags);
		return buffer;
	}

	Buffer::~Buffer()
	{
		impl->Context->GetFrameDeletionQueue().Push(impl->Buffer);
		delete impl;
	}

	void Buffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		uint32_t bufferSize = size ? size : impl->Size; // If user passes a size use it, otherwise use creation size
		memcpy((char*)impl->Buffer.Info.pMappedData + offset, data, bufferSize);
	}

	void Buffer::SetData(const Ref<GraphicsAbstraction::Buffer>& buffer)
	{
		memcpy(impl->Buffer.Info.pMappedData, buffer->impl->Buffer.Info.pMappedData, buffer->impl->Size);
	}

	void Buffer::GetData(void* data, uint32_t size, uint32_t offset) { memcpy(data, (char*)impl->Buffer.Info.pMappedData + offset, size); }
	uint32_t Buffer::GetHandle() const { return impl->Handle.GetValue(); }
	uint32_t Buffer::GetSize() const { return impl->Size; }

	Impl<Buffer>::Impl(uint32_t size, BufferUsage usage, BufferFlags flags)
		: Context(Impl<GraphicsContext>::Reference), Size(size)
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
		
		vmaCreateBuffer(Context->Allocator, &bufferInfo, &vmaAllocInfo, &Buffer.Buffer, &Buffer.Allocation, &Buffer.Info);
		if (usage & BufferUsage::StorageBuffer) UpdateDescriptor();
	}

	void Impl<Buffer>::UpdateDescriptor()
	{
		VkDescriptorBufferInfo descriptorBufferInfo = {
			.buffer = Buffer.Buffer,
			.offset = 0,
			.range = VK_WHOLE_SIZE
		};

		VkWriteDescriptorSet write = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = Context->BindlessSet,
			.dstBinding = STORAGE_BINDING,
			.dstArrayElement = Handle.GetValue(),
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.pBufferInfo = &descriptorBufferInfo
		};

		vkUpdateDescriptorSets(Context->Device, 1, &write, 0, nullptr);
	}

}