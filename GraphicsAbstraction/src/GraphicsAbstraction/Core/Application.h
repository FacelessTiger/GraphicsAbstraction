#pragma once

#include <GraphicsAbstraction/Renderer/GraphicsContext.h>
#include <GraphicsAbstraction/Renderer/Swapchain.h>
#include <GraphicsAbstraction/Renderer/CommandPool.h>
#include <GraphicsAbstraction/Renderer/CommandBuffer.h>
#include <GraphicsAbstraction/Renderer/Renderpass.h>
#include <GraphicsAbstraction/Renderer/Fence.h>
#include <GraphicsAbstraction/Renderer/Shader.h>
#include <GraphicsAbstraction/Renderer/Buffer.h>
#include <GraphicsAbstraction/Renderer/Pipeline.h>
#include <GraphicsAbstraction/Renderer/EditorCamera.h>

#include <GraphicsAbstraction/Core/Window.h>
#include <GraphicsAbstraction/Events/ApplicationEvent.h>
#include <GraphicsAbstraction/Events/KeyEvent.h>
#include <GraphicsAbstraction/Debug/Instrumentor.h>

#include <memory>

int main();

namespace GraphicsAbstraction {

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
		std::shared_ptr<GraphicsContext> m_Context;
		std::shared_ptr<Window> m_Window;

		std::shared_ptr<Swapchain> m_Swapchain;
		std::shared_ptr<CommandPool> m_CommandPool;
		std::shared_ptr<CommandBuffer> m_CommandBuffer;

		Renderpass::Specification m_RenderpassSpec;
		std::shared_ptr<Renderpass> m_Renderpass;
		std::shared_ptr<Fence> m_Fence;

		std::shared_ptr<PushConstant> m_PushConstant;
		std::shared_ptr<VertexBuffer> m_VertexBuffer;
		//std::shared_ptr<IndexBuffer> m_IndexBuffer;

		std::shared_ptr<Shader> m_QuadShader;
		std::shared_ptr<Pipeline> m_QuadPipeline;
		EditorCamera m_EditorCamera;

		bool m_Running = true;
		uint32_t m_FrameNumber = 0;

		GPUProfilerContext* m_GPUProfilerContext;
	};

}