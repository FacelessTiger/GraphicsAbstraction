#include "D3D12CommandPool.h"

namespace GraphicsAbstraction {

	Ref<CommandPool> CommandPool::Create(const Ref<Queue>& queue)
	{
		return CreateRef<D3D12CommandPool>(queue);
	}

}