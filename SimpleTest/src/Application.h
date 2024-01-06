#pragma once

#include <GraphicsAbstraction/GraphicsAbstraction.h>
#include "EditorCamera.h"

int main();

namespace GraphicsAbstraction {

	class Application
	{
	public:
		friend int ::main();

		Application();
		virtual ~Application();

		void OnEvent(Event& e);

		inline const Ref<Window> GetWindow() const { return m_Window; }

		inline static Application& Get() { return *s_Instance; }
	private:
		void Run();

		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
	private:
		static Application* s_Instance;

		Ref<Swapchain> m_Swapchain;
		Ref<Queue> m_Queue;
		Ref<Fence> m_Fence;
		Ref<Image> m_DepthImage;
		Ref<CommandPool> m_CommandPools[2];
		Ref<Shader> m_VertexShader, m_PixelShader;
		Ref<Buffer> m_VertexBuffer, m_IndexBuffer;

		bool m_Running = true;
		bool m_Minimized = false;
		uint32_t m_FrameNumber = 0;
		glm::vec2 m_ViewportSize;

		Ref<Window> m_Window;
		EditorCamera m_EditorCamera;
	};

}