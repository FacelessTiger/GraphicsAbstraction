#include "Surface.h"

#ifdef GA_RENDERER_VULKAN
	#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanSurface.h>
#endif

namespace GraphicsAbstraction {

	std::shared_ptr<Surface> Surface::Create(const std::shared_ptr<Window>& window)
	{
#ifdef GA_RENDERER_NONE
		GA_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
		return nullptr;
#elif defined(GA_RENDERER_VULKAN)
		return std::make_shared<VulkanSurface>(window);
#else
		GA_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
#endif
	}

}