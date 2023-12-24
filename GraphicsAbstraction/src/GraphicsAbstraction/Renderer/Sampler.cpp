#include "Sampler.h"

#ifdef GA_RENDERER_VULKAN
#include <Platform/GraphicsAPI/Vulkan/VulkanSampler.h>
#endif

namespace GraphicsAbstraction {

	std::shared_ptr<Sampler> Sampler::Create(Filter min, Filter mag)
	{
#ifdef GA_RENDERER_NONE
		GA_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
		return nullptr;
#elif defined(GA_RENDERER_VULKAN)
		return std::make_shared<VulkanSampler>(min, mag);
#else
		GA_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
#endif
	}

}