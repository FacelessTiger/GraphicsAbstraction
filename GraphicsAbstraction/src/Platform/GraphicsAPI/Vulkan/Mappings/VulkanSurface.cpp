#include "VulkanSurface.h"

#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanContext.h>

#include <glfw/glfw3.h>

namespace GraphicsAbstraction {

	VulkanSurface::VulkanSurface(const Ref<Window>& window)
		: m_Context(VulkanContext::GetReference())
	{
		glfwCreateWindowSurface(m_Context->Instance, (GLFWwindow*)window->GetNativeWindow(), nullptr, &Surface);
	}

	VulkanSurface::~VulkanSurface()
	{
		m_Context->GetFrameDeletionQueue().Push(Surface);
	}

}