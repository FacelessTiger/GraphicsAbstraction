#include "Pipeline.h"

#ifdef GA_RENDERER_VULKAN
	#include <Platform/GraphicsAPI/Vulkan/VulkanPipeline.h>
#endif

namespace GraphicsAbstraction {

	std::shared_ptr<Pipeline> Pipeline::Create(std::shared_ptr<GraphicsContext> context, std::shared_ptr<Shader> shader, std::shared_ptr<Renderpass> renderpass, uint32_t width, uint32_t height)
	{
#ifdef GA_RENDERER_NONE
		GA_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
		return nullptr;
#elif defined(GA_RENDERER_VULKAN)
		return std::make_shared<VulkanPipeline>(context, shader, renderpass, width, height);
#else
		GA_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
#endif
	}

}