#include "Swapchain.h"

#ifdef GA_RENDERER_VULKAN
	#include <Platform/GraphicsAPI/Vulkan/VulkanSwapchain.h>
#endif

namespace GraphicsAbstraction {

	std::shared_ptr<Swapchain> Swapchain::Create(std::shared_ptr<Window> window, std::shared_ptr<GraphicsContext> context)
	{
#ifdef GA_RENDERER_NONE
		GA_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
		return nullptr;
#elif defined(GA_RENDERER_VULKAN)
		return std::make_shared<VulkanSwapchain>(window, context);
#else
		GA_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
#endif
	}

}