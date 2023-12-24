#include "CommandPool.h"

#ifdef GA_RENDERER_VULKAN
	#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanCommandPool.h>
#endif

namespace GraphicsAbstraction {

	std::shared_ptr<CommandPool> CommandPool::Create(const std::shared_ptr<Queue>& queue)
	{
#ifdef GA_RENDERER_NONE
		GA_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
		return nullptr;
#elif defined(GA_RENDERER_VULKAN)
		return std::make_shared<VulkanCommandPool>(queue);
#else
		GA_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
#endif
	}

}