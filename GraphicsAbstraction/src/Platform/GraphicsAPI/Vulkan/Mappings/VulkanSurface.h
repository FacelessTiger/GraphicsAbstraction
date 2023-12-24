#pragma once

#include <vulkan/vulkan.h>

#include <GraphicsAbstraction/Renderer/Surface.h>
#include <GraphicsAbstraction/Core/Window.h>

#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanContext.h>

namespace GraphicsAbstraction {

	class VulkanSurface : public Surface
	{
	public:
		VkSurfaceKHR Surface;
	public:
		VulkanSurface(const std::shared_ptr<Window>& window);
		virtual ~VulkanSurface();
	private:
		VulkanContextReference m_Context;
	};

}