#include "Fence.h"

#ifdef GA_RENDERER_VULKAN
	#include <Platform/GraphicsAPI/Vulkan/VulkanFence.h>
#endif

namespace GraphicsAbstraction {

	std::shared_ptr<Fence> Fence::Create(std::shared_ptr<GraphicsContext> context)
	{
#ifdef GA_RENDERER_NONE
		std::cerr << "GA_RENDERER_NONE is currently not supported" << std::endl;
		abort();

		return nullptr;
#elif defined(GA_RENDERER_VULKAN)
		return std::make_shared<VulkanFence>(context);
#else
		std::cerr << "Unknown RendererAPI!" << std::endl;
		abort();

		return nullptr;
#endif
	}

}