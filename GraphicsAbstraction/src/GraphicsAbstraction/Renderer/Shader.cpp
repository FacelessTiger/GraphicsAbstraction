#include "Shader.h"

#ifdef GA_RENDERER_VULKAN
	#include <Platform/GraphicsAPI/Vulkan/VulkanShader.h>
#endif

namespace GraphicsAbstraction {

	std::shared_ptr<Shader> Shader::Create(std::shared_ptr<GraphicsContext> context, const std::string& filepath)
	{
#ifdef GA_RENDERER_NONE
		GA_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
		return nullptr;
#elif defined(GA_RENDERER_VULKAN)
		return std::make_shared<VulkanShader>(context, filepath);
#else
		GA_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
#endif
	}

}