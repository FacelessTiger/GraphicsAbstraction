#pragma once

#include <GraphicsAbstraction/Core/Window.h>

#include <GLFW/glfw3.h>

namespace GraphicsAbstraction {

	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		void OnUpdate() override;

		void SetSize(const glm::vec2& size) override;
		inline glm::vec2 GetSize() const override { return { m_Data.Width, m_Data.Height }; }
		inline unsigned int GetWidth() const override { return m_Data.Width; }
		inline unsigned int GetHeight() const override { return m_Data.Height; }

		void SetPosition(const glm::vec2& position) override;
		glm::vec2 GetPosition() const override;

		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }

		inline void* GetNativeWindow() const override { return m_Window; }
	private:
		GLFWwindow* m_Window;

		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};

}