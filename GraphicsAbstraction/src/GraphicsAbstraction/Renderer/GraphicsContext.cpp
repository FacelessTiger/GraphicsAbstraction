#include "GraphicsContext.h"

#ifdef GA_RENDERER_VULKAN
    #include <Platform/GraphicsAPI/Vulkan/VulkanContext.h>
#endif

#include <iostream>

namespace GraphicsAbstraction {

    std::shared_ptr<GraphicsContext> GraphicsContext::Create()
    {
#ifdef GA_RENDERER_NONE
        std::cerr << "GA_RENDERER_NONE is currently not supported" << std::endl; 
        abort(); 

        return nullptr;
#elif defined(GA_RENDERER_VULKAN)
        return std::make_shared<VulkanContext>();
#else
		std::cerr << "Unknown RendererAPI!" << std::endl;
		abort();

		return nullptr;
#endif
    }

}