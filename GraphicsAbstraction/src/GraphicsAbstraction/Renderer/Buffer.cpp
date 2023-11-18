#include "Buffer.h"

#ifdef GA_RENDERER_VULKAN
	#include <Platform/GraphicsAPI/Vulkan/VulkanBuffer.h>
#endif

namespace GraphicsAbstraction {

	std::shared_ptr<VertexBuffer> VertexBuffer::Create(std::shared_ptr<GraphicsContext> context, uint32_t size)
	{
#ifdef GA_RENDERER_NONE
		GA_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
		return nullptr;
#elif defined(GA_RENDERER_VULKAN)
		return std::make_shared<VulkanVertexBuffer>(context, size);
#else
		GA_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
#endif
	}

}