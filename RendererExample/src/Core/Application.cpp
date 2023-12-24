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
		: m_EditorCamera(30.0f, 16.0f / 9.0f, 0.1f, 10000.0f)
	{
		GA_PROFILE_SCOPE();

		GA_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = Window::Create();
		m_Window->SetEventCallback(GA_BIND_EVENT_FN(Application::OnEvent));

		Renderer::Init(m_Window);
		m_TestTexure = std::make_shared<Texture>("Assets/textures/Trickery.png");

		m_QuadProcedure = new QuadProcedure();
		//Renderer::AddProcedure(new GradientProcedure());
		Renderer::AddProcedure(m_QuadProcedure);
		#define DoQuad

		Renderer::PreProcess();
		Renderer::SetImGuiCallback([this]() {
			ImGui::Begin("Settings");

			ImGui::Text("%.3fms %ifps", m_FrameTime, (int)(1.0 / m_FrameTime * 1000.0));
			if (ImGui::Checkbox("Vsync", &m_Vsync))
				Renderer::SetVsync(m_Vsync);

#ifdef DoQuad
			if (ImGui::Button("Add quad"))
			{
				Quad data = {
					.scale = { 3.0f, 3.0f },
					.position = { -2.0f, 0.0f, 0.0f },
					.rotation = 0.0f,
					.color = { 1.0f, 0.0f, 0.0f, 1.0f }
				};

				m_QuadData[m_QuadProcedure->UploadQuad(data.position, data.scale, data.color, m_TestTexure)] = data;
			}

			for (auto& [id, data] : m_QuadData)
			{
				ImGui::PushID(id);

				if (ImGui::DragFloat3("Position", glm::value_ptr(data.position))) m_QuadProcedure->UpdateQuadPosition(id, data.position);
				if (ImGui::DragFloat2("Scale", glm::value_ptr(data.scale))) m_QuadProcedure->UpdateQuadScale(id, data.scale);
				if (ImGui::ColorEdit4("Color", glm::value_ptr(data.color))) m_QuadProcedure->UpdateQuadColor(id, data.color);
				if (ImGui::DragFloat("Rotation", &data.rotation)) m_QuadProcedure->UpdateQuadRotation(id, glm::radians(data.rotation));

				ImGui::PopID();
			}
#endif

			ImGui::End();
		});

#ifdef DoQuad
		std::vector<QuadUpload> quads;
		quads.reserve(1000 * 1000);

		for (int x = 0; x < 1000; x++)
		{
			for (int y = 0; y < 1000; y++)
			{
				glm::vec3 position(x * 0.075f, y * 0.075f, 0.0f);
				quads.push_back({
					.scale = { 0.05f, 0.05f },
					.position = position,
					.color = { 0.0f, 1.0f, 0.0f, 1.0f }
				});
			}
		}

		m_QuadProcedure->UploadQuads(quads);
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

}