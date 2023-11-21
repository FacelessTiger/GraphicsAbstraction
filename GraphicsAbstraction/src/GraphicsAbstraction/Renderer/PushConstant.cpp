#include "PushConstant.h"

#ifdef GA_RENDERER_VULKAN
	#include <Platform/GraphicsAPI/Vulkan/VulkanPushConstant.h>
#endif

namespace GraphicsAbstraction {

	std::shared_ptr<PushConstant> PushConstant::Create(uint32_t offset, uint32_t size, ShaderStage stage)
	{
#ifdef GA_RENDERER_NONE
		GA_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
		return nullptr;
#elif defined(GA_RENDERER_VULKAN)
		return std::make_shared<VulkanPushConstant>(offset, size, stage);
#else
		GA_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
#endif
	}

}