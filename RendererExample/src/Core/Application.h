#pragma once

#include <GraphicsAbstraction/Renderer/GraphicsContext.h>
#include <GraphicsAbstraction/Renderer/Surface.h>
#include <GraphicsAbstraction/Renderer/Swapchain.h>
#include <GraphicsAbstraction/Renderer/CommandPool.h>
#include <GraphicsAbstraction/Renderer/CommandBuffer.h>
#include <GraphicsAbstraction/Renderer/Shader.h>
#include <GraphicsAbstraction/Renderer/Image.h>
#include <GraphicsAbstraction/Renderer/Buffer.h>
#include <GraphicsAbstraction/Renderer/Queue.h>
#include <GraphicsAbstraction/Renderer/Fence.h>


#include <GraphicsAbstraction/ImGui/ImGuiLayer.h>
#include <GraphicsAbstraction/Core/Window.h>
#include <GraphicsAbstraction/Events/ApplicationEvent.h>
#include <GraphicsAbstraction/Events/KeyEvent.h>
#include <GraphicsAbstraction/Debug/Instrumentor.h>
#include <Renderer/Procedures/GradientProcedure.h>
#include <Renderer/EditorCamera.h>
#include <Renderer/Procedures/QuadProcedure.h>
#include <Renderer/Texture.h>

#include <memory>

int main();

namespace GraphicsAbstraction {

	struct Quad
	{
		glm::vec2 scale;
		glm::vec3 position;
		float rotation;
		glm::vec4 color;
	};

	class Application
	{
	public:
		friend int ::main();

		Application();
		virtual ~Application();

		void OnEvent(Event& e);

		inline const std::shared_ptr<Window> GetWindow() const { return m_Window; }

		inline static Application& Get() { return *s_Instance; }
	private:
		void Run();

		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
	private:
		static Application* s_Instance;

		bool m_Running = true;
		bool m_Minimized = false;
		bool m_Vsync = true;
		double m_FrameTime = 0.0;

		std::shared_ptr<Window> m_Window;
		EditorCamera m_EditorCamera;

		QuadProcedure* m_QuadProcedure;
		std::unordered_map<uint32_t, Quad> m_QuadData; // in actual engine the renderer would communicate via UUID's
		std::shared_ptr<Texture> m_TestTexure;
	};

}