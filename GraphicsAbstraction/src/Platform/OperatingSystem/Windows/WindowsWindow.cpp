#include "WindowsWindow.h"

#include <GraphicsAbstraction/Events/ApplicationEvent.h>
#include <GraphicsAbstraction/Events/KeyEvent.h>
#include <GraphicsAbstraction/Events/MouseEvent.h>

#include <GraphicsAbstraction/Debug/Instrumentor.h>
#include <GraphicsAbstraction/Core/Assert.h>

namespace GraphicsAbstraction {

	static bool s_GLFWInitialized = false;
	static unsigned int s_WindowCount = 0;

	static void GLFWErrorCallback(int error, const char* description)
	{
		GA_CORE_ERROR("GLFW Eror ({0}): {1}", error, description);
	}

	Ref<Window> Window::Create(const WindowProps& props)
	{
		return CreateRef<WindowsWindow>(props);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		GA_PROFILE_SCOPE();

		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;

		if (!s_GLFWInitialized)
		{
			int result = glfwInit();
			GA_CORE_ASSERT(result == GLFW_TRUE, "Could not initialize GLFW!");	

			glfwSetErrorCallback(GLFWErrorCallback);
			s_GLFWInitialized = true;
		}

		// if requested window size is higher than monitor size then set it to monitor size - 10;
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		if (m_Data.Width > (uint32_t)mode->width) m_Data.Width = (uint32_t)mode->width - 10;
		if (m_Data.Height > (uint32_t)mode->height) m_Data.Height = (uint32_t)mode->height - 10;

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		if (props.CustomTitlebar) glfwWindowHint(GLFW_TITLEBAR, false);
		m_Window = glfwCreateWindow((int)m_Data.Width, (int)m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);
		s_WindowCount++;
		
		glfwSetWindowUserPointer(m_Window, &m_Data);

		// Set GLFW callbacks
		glfwSetTitlebarHitTestCallback(m_Window, [](GLFWwindow*, int x, int y, int* hit)
		{
			*hit = true;
		});

		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			WindowCloseEvent event;
			data.EventCallback(event);
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key);
					data.EventCallback(event);
					break;
				}

				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					data.EventCallback(event);
					break;
				}

				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, true);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(button);
					data.EventCallback(event);
					break;
				}

				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(button);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.EventCallback(event);
		});
	}

	WindowsWindow::~WindowsWindow()
	{
		glfwDestroyWindow(m_Window);
		s_WindowCount--;

		if (s_WindowCount == 0)
		{
			glfwTerminate();
			s_GLFWInitialized = false;
		}
	}

	void WindowsWindow::OnUpdate()
	{
		GA_PROFILE_SCOPE();
		glfwPollEvents();
	}

}