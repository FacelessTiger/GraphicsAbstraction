#pragma once

#include <GraphicsAbstraction/Renderer/GraphicsContext.h>
#include <GraphicsAbstraction/Renderer/Swapchain.h>
#include <GraphicsAbstraction/Renderer/CommandPool.h>
#include <GraphicsAbstraction/Renderer/CommandBuffer.h>
#include <GraphicsAbstraction/Renderer/Renderpass.h>
#include <GraphicsAbstraction/Renderer/Fence.h>
#include <GraphicsAbstraction/Renderer/Shader.h>

#include <GraphicsAbstraction/Core/Window.h>

#include <GraphicsAbstraction/Events/ApplicationEvent.h>

#include <memory>

int main();

namespace GraphicsAbstraction {

	class Application
	{
	public:
		friend int ::main();

		Application();

		void OnEvent(Event& e);
	private:
		void Run();

		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
	private:
		std::shared_ptr<GraphicsContext> m_Context;
		std::shared_ptr<Window> m_Window;

		std::shared_ptr<Swapchain> m_Swapchain;
		std::shared_ptr<CommandPool> m_CommandPool;
		std::shared_ptr<CommandBuffer> m_CommandBuffer;

		std::shared_ptr<Renderpass> m_Renderpass;
		std::shared_ptr<Fence> m_Fence;
		std::shared_ptr<Shader> m_TriangleShader;

		bool m_Running = true;
		uint32_t m_FrameNumber = 0;
	};

}