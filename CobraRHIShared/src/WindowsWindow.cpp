#define GA_DLL_LINK
#include <GraphicsAbstraction/Core/Window.h>
#include <GraphicsAbstraction/Events/ApplicationEvent.h>
#include <GraphicsAbstraction/Events/KeyEvent.h>
#include <GraphicsAbstraction/Events/MouseEvent.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3.h>
#include <glfw/glfw3native.h>

namespace GraphicsAbstraction {

	static bool s_GLFWInitialized = false;
	static unsigned int s_WindowCount = 0;

	template<>
	struct Impl<Window>
	{
		GLFWwindow* Window;
		HWND Hwnd;
		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;

			Window::EventCallbackFn EventCallback;
		};

		WindowData Data;

		void Create(const WindowProps& props);
	};

	Ref<Window> Window::Create(const WindowProps& props)
	{
		auto window = CreateRef<Window>();
		window->impl = new Impl<Window>;
		window->impl->Create(props);
		return window;
	}

	Window::~Window()
	{
		glfwDestroyWindow(impl->Window);
		s_WindowCount--;

		if (s_WindowCount == 0)
		{
			glfwTerminate();
			s_GLFWInitialized = false;
		}
	}

	void Window::OnUpdate()
	{
		glfwPollEvents();
	}

	void Window::SetSize(const glm::vec2& size)
	{
		impl->Data.Width = (uint32_t)size.x;
		impl->Data.Height = (uint32_t)size.y;

		glfwSetWindowSize(impl->Window, impl->Data.Width, impl->Data.Height);
	}

	glm::vec2 Window::GetSize() const { return { impl->Data.Width, impl->Data.Height }; }
	unsigned int Window::GetWidth() const { return impl->Data.Width; }
	unsigned int Window::GetHeight() const { return impl->Data.Height; }

	void Window::SetPosition(const glm::vec2& position)
	{
		glfwSetWindowPos(impl->Window, (int)position.x, (int)position.y);
	}

	glm::vec2 Window::GetPosition() const
	{
		int x, y;
		glfwGetWindowPos(impl->Window, &x, &y);

		return glm::vec2(x, y);
	}

	glm::vec2 Window::GetMousePosition() const 
	{ 
		double xpos, ypos;
		glfwGetCursorPos(impl->Window, &xpos, &ypos);
		return { (float)xpos, (float)ypos };
	}

	bool Window::IsKeyPressed(int keycode) const { return glfwGetKey(impl->Window, keycode) == GLFW_PRESS; }
	bool Window::IsMouseButtonPressed(int button) const { return glfwGetMouseButton(impl->Window, button) == GLFW_PRESS; }

	void Window::SetEventCallback(const EventCallbackFn& callback) { impl->Data.EventCallback = callback; }
	void* Window::GetNativeWindow() const { return impl->Hwnd; }

	void Impl<Window>::Create(const WindowProps& props)
	{
		Data.Title = props.Title;
		Data.Width = props.Width;
		Data.Height = props.Height;

		if (!s_GLFWInitialized)
		{
			int result = glfwInit();
			assert((result == GLFW_TRUE) && "Could not initialize GLFW!");	
			s_GLFWInitialized = true;
		}

		// if requested window size is higher than monitor size then set it to monitor size - 10;
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		if (Data.Width > (uint32_t)mode->width) Data.Width = (uint32_t)mode->width - 10;
		if (Data.Height > (uint32_t)mode->height) Data.Height = (uint32_t)mode->height - 10;

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		if (props.CustomTitlebar) glfwWindowHint(GLFW_TITLEBAR, false);
		Window = glfwCreateWindow((int)Data.Width, (int)Data.Height, Data.Title.c_str(), nullptr, nullptr);
		Hwnd = glfwGetWin32Window(Window);
		s_WindowCount++;
		
		glfwSetWindowUserPointer(Window, &Data);

		// Set GLFW callbacks
		glfwSetTitlebarHitTestCallback(Window, [](GLFWwindow*, int x, int y, int* hit)
		{
			*hit = true;
		});

		glfwSetWindowSizeCallback(Window, [](GLFWwindow* window, int width, int height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
		});

		glfwSetWindowCloseCallback(Window, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			WindowCloseEvent event;
			data.EventCallback(event);
		});

		glfwSetKeyCallback(Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
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

		glfwSetCharCallback(Window, [](GLFWwindow* window, unsigned int key)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			KeyTypedEvent event(key);
			data.EventCallback(event);
		});

		glfwSetMouseButtonCallback(Window, [](GLFWwindow* window, int button, int action, int mods)
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

		glfwSetScrollCallback(Window, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
		});

		glfwSetCursorPosCallback(Window, [](GLFWwindow* window, double xPos, double yPos)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.EventCallback(event);
		});
	}

}