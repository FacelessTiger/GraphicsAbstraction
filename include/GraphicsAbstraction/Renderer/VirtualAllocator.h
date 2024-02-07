#pragma once

#include <GraphicsAbstraction/Core/Core.h>

namespace GraphicsAbstraction {

	struct GA_DLL_LINK VirtualAllocation
	{
		Impl<VirtualAllocation>* impl;
		uint64_t Offset;

		VirtualAllocation() = default;
		VirtualAllocation(const VirtualAllocation&) = delete;
		VirtualAllocation(VirtualAllocation&& other)
		{
			impl = other.impl;
			Offset = other.Offset;

			other.impl = nullptr;
		}
		virtual ~VirtualAllocation();

		VirtualAllocation& operator=(VirtualAllocation&& other)
		{
			impl = other.impl;
			Offset = other.Offset;

			other.impl = nullptr;
			return *this;
		}
	};

	class GA_DLL_LINK VirtualAllocator : public RefCounted
	{
	public:
		VirtualAllocation Allocate(uint32_t size);
		void Free(VirtualAllocation allocation);

		GA_RHI_TEMPLATE(VirtualAllocator, uint32_t size);
	};

}