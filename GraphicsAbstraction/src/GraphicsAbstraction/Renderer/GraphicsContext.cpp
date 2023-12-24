#include "GraphicsContext.h"

#ifdef GA_RENDERER_VULKAN
    #include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanContext.h>
#endif

namespace GraphicsAbstraction {

    std::unique_ptr<GraphicsContext> GraphicsContext::s_Instance = nullptr;

    void GraphicsContext::Init(uint32_t frameInFlightCount)
    {
#ifdef GA_RENDERER_NONE
        GA_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
#elif defined(GA_RENDERER_VULKAN)
        s_Instance = std::make_unique<VulkanContext>(frameInFlightCount);
#else
        GA_CORE_ASSERT(false, "Unknown RendererAPI!");
#endif
    }

}