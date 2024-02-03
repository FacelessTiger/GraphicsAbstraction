#pragma once

#include <GraphicsAbstraction/GraphicsAbstraction.h>
#include <Renderer/EditorCamera.h>
#include <Panels/SceneHierarchyPanel.h>
#include <Panels/AssetPanel.h>
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

		inline const Ref<Window> GetWindow() const { return m_Window; }

		inline static Application& Get() { return *s_Instance; }
	private:
		void Run();
		void LoadGLTF(const std::filesystem::path path);

		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
		void OnImGuiRender();
	private:
		static Application* s_Instance;

		bool m_Running = true;
		bool m_Minimized = false;
		bool m_Vsync = true;
		glm::vec2 m_ViewportSize;
		double m_FrameTime = 0.0;

		Ref<Window> m_Window;
		Ref<Scene> m_Scene;
		EditorCamera m_EditorCamera;

		AssetHandle m_WhiteTexture;
		SceneHierarchyPanel m_SceneHierarchyPanel;
		AssetPanel m_AssetPanel;
	};

}