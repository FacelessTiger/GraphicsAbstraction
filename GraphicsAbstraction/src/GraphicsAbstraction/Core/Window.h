#pragma once

#include <string>
#include <memory>

namespace GraphicsAbstraction {

	struct WindowProps
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;

		WindowProps(const std::string& title = "Graphics Abstraction",
			uint32_t width = 1600,
			uint32_t height = 900)
			: Title(title), Width(width), Height(height)
		{ }
	};

	class Window
	{
	public:
		virtual ~Window() { }

		virtual void OnUpdate() = 0;

		virtual bool ShouldClose() const = 0; // In a proper engine, this would be handled through a WindowClose event instead, but I'm just focusing on graphics abstraction here

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual void* GetNativeWindow() const = 0;

		static std::shared_ptr<Window> Create(const WindowProps& props = WindowProps());
	};

}