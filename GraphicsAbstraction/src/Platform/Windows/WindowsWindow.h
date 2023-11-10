#pragma once

#include <GraphicsAbstraction/Core/Window.h>
#include <GraphicsAbstraction/Renderer/GraphicsContext.h>

#include <GLFW/glfw3.h>

namespace GraphicsAbstraction {

	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		void OnUpdate() override;

		inline bool ShouldClose() const override { return m_Data.ShouldClose; }

		inline unsigned int GetWidth() const override { return m_Data.Width; }
		inline unsigned int GetHeight() const override { return m_Data.Height; }

		inline void* GetNativeWindow() const override { return m_Window; }
	private:
		GLFWwindow* m_Window;

		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool ShouldClose;
		};

		WindowData m_Data;
	};

}