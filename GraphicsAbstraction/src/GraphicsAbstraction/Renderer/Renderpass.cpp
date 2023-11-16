#include "Renderpass.h"

#ifdef GA_RENDERER_VULKAN
	#include <Platform/GraphicsAPI/Vulkan/VulkanRenderpass.h>
#endif

namespace GraphicsAbstraction {

	std::shared_ptr<GraphicsAbstraction::Renderpass> Renderpass::Create(const Specification& spec, std::shared_ptr<GraphicsContext> context, std::shared_ptr<Swapchain> swapchain)
	{
#ifdef GA_RENDERER_NONE
		GA_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
		return nullptr;
#elif defined(GA_RENDERER_VULKAN)
		return std::make_shared<VulkanRenderpass>(spec, context, swapchain);
#else
		GA_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
#endif
	}

}