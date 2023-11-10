#include "GraphicsContext.h"

#include <GraphicsAbstraction/Renderer/Renderer.h>
#include <Platform/Vulkan/VulkanContext.h>

#include <iostream>

namespace GraphicsAbstraction {

    std::shared_ptr<GraphicsContext> GraphicsContext::Create()
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:    std::cerr << "RendererAPI::None is currently not supported" << std::endl; abort(); return nullptr;
            case RendererAPI::API::Vulkan:  return std::make_shared<VulkanContext>();
        }

        std::cerr << "Unknown RendererAPI!" << std::endl;
        abort();

        return nullptr;
    }

}