#include "Renderpass.h"

#ifdef GA_RENDERER_VULKAN
	#include <Platform/GraphicsAPI/Vulkan/VulkanRenderpass.h>
#endif

namespace GraphicsAbstraction {

	std::shared_ptr<GraphicsAbstraction::Renderpass> Renderpass::Create(std::shared_ptr<GraphicsContext> context, const Specification& spec)
	{
#ifdef GA_RENDERER_NONE
		GA_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
		return nullptr;
#elif defined(GA_RENDERER_VULKAN)
		return std::make_shared<VulkanRenderpass>(context, spec);
#else
		GA_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
#endif
	}

}