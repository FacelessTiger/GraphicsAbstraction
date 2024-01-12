#pragma once

#include <GraphicsAbstraction/Events/Event.h>
#include <GraphicsAbstraction/Core/Core.h>

#include <string>
#include <memory>
#include <functional>
#include <glm/glm.hpp>

namespace GraphicsAbstraction {

	struct WindowProps
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;
		bool CustomTitlebar;

		WindowProps(const std::string& title = "Graphics Abstraction",
			uint32_t width = 1600,
			uint32_t height = 900, 
			bool customTitlebar = false)
			: Title(title), Width(width), Height(height), CustomTitlebar(customTitlebar)
		{ }
	};

	class Window : public RefCounted
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() { }

		virtual void OnUpdate() = 0;

		virtual void SetSize(const glm::vec2& size) = 0;
		virtual glm::vec2 GetSize() const = 0;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual void SetPosition(const glm::vec2& position) = 0;
		virtual glm::vec2 GetPosition() const = 0;

		virtual inline void SetEventCallback(const EventCallbackFn& callback) = 0;

		virtual void* GetNativeWindow() const = 0;

		static Ref<Window> Create(const WindowProps& props = WindowProps());
	};

}