#include "Renderpass.h"

#ifdef GA_RENDERER_VULKAN
	#include <Platform/GraphicsAPI/Vulkan/VulkanRenderpass.h>
#endif

namespace GraphicsAbstraction {

	std::shared_ptr<GraphicsAbstraction::Renderpass> Renderpass::Create(const Specification& spec, std::shared_ptr<GraphicsContext> context, std::shared_ptr<Swapchain> swapchain)
	{
#ifdef GA_RENDERER_NONE
		std::cerr << "GA_RENDERER_NONE is currently not supported" << std::endl;
		abort();

		return nullptr;
#elif defined(GA_RENDERER_VULKAN)
		return std::make_shared<VulkanRenderpass>(spec, context, swapchain);
#else
		std::cerr << "Unknown RendererAPI!" << std::endl;
		abort();

		return nullptr;
#endif
	}

}