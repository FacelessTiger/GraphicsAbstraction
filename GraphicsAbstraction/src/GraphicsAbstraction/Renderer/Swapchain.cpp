#include "Swapchain.h"

#include <GraphicsAbstraction/Renderer/Renderer.h>
#include <Platform/Vulkan/VulkanSwapchain.h>

#include <iostream>

namespace GraphicsAbstraction {

	std::shared_ptr<Swapchain> Swapchain::Create(std::shared_ptr<Window> window, std::shared_ptr<GraphicsContext> context)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:    std::cerr << "RendererAPI::None is currently not supported" << std::endl; abort(); return nullptr;
			case RendererAPI::API::Vulkan:  return std::make_shared<VulkanSwapchain>(window, context);
		}

		std::cerr << "Unknown RendererAPI!" << std::endl;
		abort();

		return nullptr;
	}

}