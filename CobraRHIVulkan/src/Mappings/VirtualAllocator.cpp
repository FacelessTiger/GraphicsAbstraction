#include <VulkanRHI.h>

namespace GraphicsAbstraction {

	Ref<VirtualAllocator> VirtualAllocator::Create(uint32_t size)
	{
		auto buffer = CreateRef<VirtualAllocator>();
		buffer->impl = new Impl<VirtualAllocator>(size);
		return buffer;
	}

	VirtualAllocator::~VirtualAllocator()
	{
		vmaClearVirtualBlock(impl->Block);
		vmaDestroyVirtualBlock(impl->Block);
		delete impl;
	}

	VirtualAllocation::~VirtualAllocation()
	{
		delete impl;
	}

	VirtualAllocation VirtualAllocator::Allocate(uint32_t size)
	{
		VmaVirtualAllocationCreateInfo info = { .size = size };

		VirtualAllocation ret;
		VmaVirtualAllocation allocation;
		VK_CHECK(vmaVirtualAllocate(impl->Block, &info, &allocation, &ret.Offset));

		ret.impl = new Impl<VirtualAllocation>(allocation);
		return std::move(ret);
	}

	void VirtualAllocator::Free(VirtualAllocation allocation)
	{
		vmaVirtualFree(impl->Block, allocation.impl->Allocation);
	}

	Impl<VirtualAllocation>::Impl(VmaVirtualAllocation allocation)
		: Allocation(allocation)
	{ }

	Impl<VirtualAllocator>::Impl(uint32_t size)
	{
		VmaVirtualBlockCreateInfo info = { .size = size };
		VK_CHECK(vmaCreateVirtualBlock(&info, &Block));
	}

}