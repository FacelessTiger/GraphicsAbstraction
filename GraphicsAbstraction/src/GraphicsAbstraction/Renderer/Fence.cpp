#include "Fence.h"

#ifdef GA_RENDERER_VULKAN
	#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanFence.h>
#endif

namespace GraphicsAbstraction {

	std::shared_ptr<Fence> Fence::Create()
	{
#ifdef GA_RENDERER_NONE
		GA_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
		return nullptr;
#elif defined(GA_RENDERER_VULKAN)
		return std::make_shared<VulkanFence>();
#else
		GA_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
#endif
	}

}