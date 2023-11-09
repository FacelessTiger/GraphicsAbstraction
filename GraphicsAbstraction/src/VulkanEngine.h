#pragma once

#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace VAP {

	class VulkanEngine
	{
	public:
		void Init();
		void Run();
		void Cleanup();
	private:
		void Draw();
	private:
		bool m_IsInitialized = false;
		unsigned int m_FrameNumber;

		VkExtent2D m_WindowExtent = { 1700, 900 };
		GLFWwindow* m_Window;
	};

}