#include "D3D12Fence.h"

namespace GraphicsAbstraction {

	Ref<Fence> Fence::Create()
	{
		return CreateRef<D3D12Fence>();
	}

}