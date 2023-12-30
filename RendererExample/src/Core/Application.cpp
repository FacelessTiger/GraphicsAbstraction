#include "Application.h"

#include <GraphicsAbstraction/Core/Core.h>
#include <Renderer/Renderer.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/gtx//transform.hpp>

//#define TINYOBJLOADER_IMPLEMENTATION
//#include <tiny_obj_loader.h>

#include <unordered_map>
#include <filesystem>
#include <chrono>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace GraphicsAbstraction {

	Application* Application::s_Instance = nullptr;

	Application::Application()
		: m_EditorCamera(70.0f, 16.0f / 9.0f, 0.1f)
	{
		GA_PROFILE_SCOPE();

		GA_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = Window::Create();
		m_Window->SetEventCallback(GA_BIND_EVENT_FN(Application::OnEvent));

		Renderer::Init(m_Window, true);
		m_TestTexure = std::make_shared<Texture>("Assets/textures/Trickery.png");

		//m_QuadProcedure = new QuadProcedure();
		Renderer::AddProcedure(new GradientProcedure());
		//Renderer::AddProcedure(m_QuadProcedure);
		//#define DoQuad

		Renderer::PreProcess();
		Renderer::SetImGuiCallback(GA_BIND_EVENT_FN(Application::OnImGuiRender));

#ifdef DoQuad
		for (int x = 0; x < 1000; x++)
		{
			for (int y = 0; y < 1000; y++)
			{
				glm::vec3 position(x * 0.075f, y * 0.075f, 0.0f);
				m_QuadProcedure->UploadQuad(position, { 0.05f, 0.05f }, { 0.0f, 1.0f, 0.0f, 1.0f }, nullptr);
			}
		}
#endif
	}

	Application::~Application()
	{
		GA_PROFILE_SCOPE();
		Renderer::Shutdown();
	}

	void Application::OnEvent(Event& e)
	{
		m_EditorCamera.OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(GA_BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(GA_BIND_EVENT_FN(Application::OnWindowResize));
	}

	void Application::Run()
	{
		while (m_Running)
		{
			GA_PROFILE_SCOPE();
			auto t1 = std::chrono::high_resolution_clock::now();

			if (!m_Minimized)
			{
				Renderer::Render(m_EditorCamera);
			}

			m_Window->OnUpdate();
			m_EditorCamera.OnUpdate();

			auto t2 = std::chrono::high_resolution_clock::now();
			m_FrameTime = std::chrono::duration<double, std::milli>(t2 - t1).count();
			GA_FRAME_MARK();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return true;
		}

		m_Minimized = false;
		Renderer::Resize(e.GetWidth(), e.GetHeight());

		return true;
	}

	void Application::OnImGuiRender()
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
		const ImGuiViewport* viewport = ImGui::GetMainViewport();

		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(1.0f, 1.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);

		ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f });
		ImGui::Begin("DockSpace Demo", nullptr, window_flags);
		ImGui::PopStyleColor();

		ImGui::PopStyleVar(4);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 400.0f;
		ImGui::DockSpace(ImGui::GetID("MyDockspace"));
		style.WindowMinSize.x = minWinSizeX;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
		ImGui::Begin("Viewport");

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		if (m_ViewportSize.x != viewportPanelSize.x || m_ViewportSize.y != viewportPanelSize.y)
		{
			m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
			m_EditorCamera.SetViewportSize(viewportPanelSize.x, viewportPanelSize.y);
		}
		ImGui::Image((ImTextureID)(uint64_t)Renderer::GetDrawImage()->GetHandle(), { viewportPanelSize.x, viewportPanelSize.y });

		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::Begin("Settings");
		ImGui::Text("%.3fms %ifps", m_FrameTime, (int)(1.0 / m_FrameTime * 1000.0));
		if (ImGui::Checkbox("Vsync", &m_Vsync))
			Renderer::SetVsync(m_Vsync);

		ImGui::End();

		ImGui::End();
	}

}