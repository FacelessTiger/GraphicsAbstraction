#include "Swapchain.h"

#ifdef GA_RENDERER_VULKAN
	#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanSwapchain.h>
#endif

namespace GraphicsAbstraction {

	std::shared_ptr<Swapchain> Swapchain::Create(const std::shared_ptr<Surface>& surface, const glm::vec2& size, bool enableVSync)
	{
#ifdef GA_RENDERER_NONE
		GA_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
		return nullptr;
#elif defined(GA_RENDERER_VULKAN)
		return std::make_shared<VulkanSwapchain>(surface, size, enableVSync);
#else
		GA_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
#endif
	}

}