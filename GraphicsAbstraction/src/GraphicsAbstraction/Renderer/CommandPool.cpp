#include "CommandPool.h"

#ifdef GA_RENDERER_VULKAN
	#include <Platform/GraphicsAPI/Vulkan/VulkanCommandPool.h>
#endif

namespace GraphicsAbstraction {

	std::shared_ptr<GraphicsAbstraction::CommandPool> CommandPool::Create(std::shared_ptr<GraphicsContext> context, QueueType type)
	{
#ifdef GA_RENDERER_NONE
		std::cerr << "GA_RENDERER_NONE is currently not supported" << std::endl;
		abort();

		return nullptr;
#elif defined(GA_RENDERER_VULKAN)
		return std::make_shared<VulkanCommandPool>(context, type);
#else
		std::cerr << "Unknown RendererAPI!" << std::endl;
		abort();

		return nullptr;
#endif
	}

}