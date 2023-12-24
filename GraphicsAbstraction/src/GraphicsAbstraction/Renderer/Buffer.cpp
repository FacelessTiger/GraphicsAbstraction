#include "Buffer.h"

#ifdef GA_RENDERER_VULKAN
	#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanBuffer.h>
#endif

namespace GraphicsAbstraction {

	std::shared_ptr<Buffer> Buffer::Create(uint32_t size, BufferUsage usage, BufferFlags flags)
	{
#ifdef GA_RENDERER_NONE
		GA_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
		return nullptr;
#elif defined(GA_RENDERER_VULKAN)
		return std::make_shared<VulkanBuffer>(size, usage, flags);
#else
		GA_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
#endif
	}

}