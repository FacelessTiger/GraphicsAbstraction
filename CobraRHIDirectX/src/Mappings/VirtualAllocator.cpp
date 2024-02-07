#include <DirectXRHI.h>

namespace GraphicsAbstraction {

	Ref<VirtualAllocator> VirtualAllocator::Create(uint32_t size)
	{
		auto buffer = CreateRef<VirtualAllocator>();
		buffer->impl = new Impl<VirtualAllocator>(size);
		return buffer;
	}

	VirtualAllocator::~VirtualAllocator()
	{
		impl->Block->Clear();
		delete impl;
	}

	VirtualAllocation::~VirtualAllocation()
	{
		delete impl;
	}

	VirtualAllocation VirtualAllocator::Allocate(uint32_t size)
	{
		D3D12MA::VIRTUAL_ALLOCATION_DESC desc = { .Size = size };

		VirtualAllocation ret;
		D3D12MA::VirtualAllocation allocation;
		impl->Block->Allocate(&desc, &allocation, &ret.Offset);

		ret.impl = new Impl<VirtualAllocation>(allocation);
		return ret;
	}

	void VirtualAllocator::Free(VirtualAllocation allocation)
	{
		impl->Block->FreeAllocation(allocation.impl->Allocation);
	}

	Impl<VirtualAllocation>::Impl(D3D12MA::VirtualAllocation allocation)
		: Allocation(allocation)
	{ }

	Impl<VirtualAllocator>::Impl(uint32_t size)
	{
		D3D12MA::VIRTUAL_BLOCK_DESC desc = { .Size = size };
		D3D12_CHECK(D3D12MA::CreateVirtualBlock(&desc, &Block));
	}

}