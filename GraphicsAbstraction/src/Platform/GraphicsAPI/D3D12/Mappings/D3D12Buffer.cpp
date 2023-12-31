#include "D3D12Buffer.h"

namespace GraphicsAbstraction {

	Ref<Buffer> Buffer::Create(uint32_t size, BufferUsage usage, BufferFlags flags)
	{
		return CreateRef<D3D12Buffer>(size, usage, flags);
	}

}