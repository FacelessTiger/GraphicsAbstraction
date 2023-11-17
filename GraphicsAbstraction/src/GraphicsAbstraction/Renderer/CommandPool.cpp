#include "CommandPool.h"

#ifdef GA_RENDERER_VULKAN
	#include <Platform/GraphicsAPI/Vulkan/VulkanCommandPool.h>
#endif

namespace GraphicsAbstraction {

	std::shared_ptr<GraphicsAbstraction::CommandPool> CommandPool::Create(std::shared_ptr<GraphicsContext> context, QueueType type)
	{
#ifdef GA_RENDERER_NONE
		GA_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
		return nullptr;
#elif defined(GA_RENDERER_VULKAN)
		return std::make_shared<VulkanCommandPool>(context, type);
#else
		GA_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
#endif
	}

}