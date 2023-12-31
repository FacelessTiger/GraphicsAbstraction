#include "D3D12Swapchain.h"

namespace GraphicsAbstraction {

	Ref<Swapchain> Swapchain::Create(Ref<Surface>& surface, const glm::vec2& size, bool enableVSync)
	{
		return CreateRef<D3D12Swapchain>(surface, size, enableVSync);
	}

}