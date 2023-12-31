#include "D3D12Surface.h"

namespace GraphicsAbstraction {

	Ref<Surface> Surface::Create(const Ref<Window>& window)
	{
		return CreateRef<D3D12Surface>(window);
	}

}