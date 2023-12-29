#include "CommandPool.h"

#ifdef GA_RENDERER_VULKAN
	#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanCommandPool.h>
#endif

namespace GraphicsAbstraction {

	Ref<CommandPool> CommandPool::Create(const Ref<Queue>& queue)
	{
#ifdef GA_RENDERER_NONE
		GA_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
		return nullptr;
#elif defined(GA_RENDERER_VULKAN)
		return CreateRef<VulkanCommandPool>(queue);
#else
		GA_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
#endif
	}

}