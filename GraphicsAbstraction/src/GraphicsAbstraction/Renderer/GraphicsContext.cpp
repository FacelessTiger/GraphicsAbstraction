#include "GraphicsContext.h"

#ifdef GA_RENDERER_VULKAN
    #include <Platform/GraphicsAPI/Vulkan/VulkanContext.h>
#endif

namespace GraphicsAbstraction {

    std::shared_ptr<GraphicsContext> GraphicsContext::Create()
    {
#ifdef GA_RENDERER_NONE
        GA_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
        return nullptr;
#elif defined(GA_RENDERER_VULKAN)
        return std::make_shared<VulkanContext>();
#else
        GA_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
#endif
    }

}