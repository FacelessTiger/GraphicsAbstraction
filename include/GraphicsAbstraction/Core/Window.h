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

	struct Window : public RefCounted
	{
		using EventCallbackFn = std::function<void(Event&)>;
		void OnUpdate();

		void SetSize(const glm::vec2& size);
		glm::vec2 GetSize() const;
		uint32_t GetWidth() const;
		uint32_t GetHeight() const;

		void SetPosition(const glm::vec2& position);
		glm::vec2 GetPosition() const;

		glm::vec2 GetMousePosition() const;
		bool IsKeyPressed(int keycode) const;
		bool IsMouseButtonPressed(int button) const;

		void SetEventCallback(const EventCallbackFn& callback);
		void* GetNativeWindow() const;

		GA_RHI_TEMPLATE(Window, const WindowProps& props = WindowProps());
	};

}