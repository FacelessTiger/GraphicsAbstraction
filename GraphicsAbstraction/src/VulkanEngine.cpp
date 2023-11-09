#include "VulkanEngine.h"

#include <GLFW/glfw3.h>

namespace VAP {

	void VulkanEngine::Init()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_Window = glfwCreateWindow(m_WindowExtent.width, m_WindowExtent.height, "Vulkan Engine", nullptr, nullptr);

		m_IsInitialized = true;
	}

	void VulkanEngine::Run()
	{
		while (!glfwWindowShouldClose(m_Window))
		{
			glfwPollEvents();

			Draw();
		}
	}

	void VulkanEngine::Cleanup()
	{
		if (m_IsInitialized)
		{
			glfwDestroyWindow(m_Window);
			glfwTerminate();
		}
	}

	void VulkanEngine::Draw()
	{
	}

}