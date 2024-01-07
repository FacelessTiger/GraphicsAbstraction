#include "Application.h"

#include <unordered_map>
#include <filesystem>
#include <chrono>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace GraphicsAbstraction {

	struct Vertex
	{
		glm::vec2 position;
		glm::vec2 uv;
	};

	struct PushConstant
	{
		glm::mat4 projection;
		uint32_t vertices;
		uint32_t image;
		uint32_t sampler;
	};

	Application* Application::s_Instance = nullptr;

	Application::Application()
		: m_EditorCamera(70.0f, 16.0f / 9.0f, 0.1f)
	{
		GA_PROFILE_SCOPE();

		GA_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = Window::Create();
		m_Window->SetEventCallback(GA_BIND_EVENT_FN(Application::OnEvent));

		GraphicsContext::Init(2);

		m_Swapchain = Swapchain::Create(m_Window, m_Window->GetSize());
		m_Queue = GraphicsContext::GetQueue(QueueType::Graphics);
		m_Fence = Fence::Create();

		m_VertexShader = Shader::Create("Assets/shaders/simpleVertex.hlsl", ShaderStage::Vertex);
		m_PixelShader = Shader::Create("Assets/shaders/simplePixel.hlsl", ShaderStage::Pixel);
		m_DepthImage = Image::Create(m_Window->GetSize(), ImageFormat::D32_SFLOAT, ImageUsage::DepthStencilAttachment);

		for (int i = 0; i < 2; i++)
			m_CommandPools[i] = CommandPool::Create(m_Queue);

		// vertex/index buffer
		std::vector<Vertex> vertices = {
			{ {  0.5f, -0.5f }, { 1, 0 } },
			{ {  0.5f,  0.5f }, { 1, 1 } },
			{ { -0.5f, -0.5f }, { 0, 0 } },
			{ { -0.5f,  0.5f }, { 0, 1 } }
		};
		std::vector<uint16_t> indices = { 0, 1, 2, 2, 1, 3 };
		uint32_t verticesSize = vertices.size() * sizeof(Vertex);
		uint32_t indicesSize = indices.size() * sizeof(uint16_t);

		m_VertexBuffer = Buffer::Create(verticesSize, BufferUsage::StorageBuffer | BufferUsage::TransferDst, BufferFlags::DeviceLocal);
		m_IndexBuffer = Buffer::Create(indicesSize, BufferUsage::IndexBuffer | BufferUsage::TransferDst, BufferFlags::DeviceLocal);

		// texture
		int width, height, channels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load("Assets/textures/trickery.png", &width, &height, &channels, 4);

		uint32_t imageSize = width * height * 4;
		m_Image = Image::Create({ width, height }, ImageFormat::R8G8B8A8_UNORM, ImageUsage::Sampled | ImageUsage::TransferDst);
		m_Sampler = Sampler::Create(Filter::Linear, Filter::Linear);

		// staging
		auto staging = Buffer::Create(verticesSize + indicesSize + imageSize, BufferUsage::TransferSrc, BufferFlags::Mapped);
		staging->SetData(vertices.data(), verticesSize);
		staging->SetData(indices.data(), indicesSize, verticesSize);
		staging->SetData(data, imageSize, verticesSize + indicesSize);

		auto cmd = m_CommandPools[0]->Reset()->Begin();
		cmd->CopyToBuffer(staging, m_VertexBuffer, verticesSize);
		cmd->CopyToBuffer(staging, m_IndexBuffer, indicesSize, verticesSize);
		cmd->CopyToImage(staging, m_Image, verticesSize + indicesSize);
		m_Queue->Submit(cmd, nullptr, m_Fence);
		m_Fence->Wait();

		ImGuiLayer::Init(m_CommandPools[0], m_Swapchain, m_Window, m_Queue, m_Fence);
	}

	Application::~Application()
	{
		GA_PROFILE_SCOPE();
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

			if (!m_Minimized)
			{
				ImGuiLayer::BeginFrame();

				ImGui::Begin("Test");
				ImGui::Text("Heck!!");
				ImGui::Image((ImTextureID)(uint64_t)m_Image->GetHandle(), { 200, 200 }, { 0, 1 }, { 1, 0 });
				ImGui::End();

				uint32_t fif = m_FrameNumber++ % 2;
				m_Fence->Wait();
				GraphicsContext::SetFrameInFlight(fif);
				m_Queue->Acquire(m_Swapchain, m_Fence);

				auto cmd = m_CommandPools[fif]->Reset()->Begin();
				cmd->Clear(m_Swapchain->GetCurrent(), { 0.0f, 0.0f, 1.0f, 1.0f });

				cmd->BindShaders({ m_VertexShader, m_PixelShader });
				cmd->SetDepthTest(true, true, CompareOperation::GreaterEqual);

				cmd->BeginRendering(m_Window->GetSize(), { m_Swapchain->GetCurrent() }, m_DepthImage);

				cmd->SetViewport(m_Window->GetSize());
				cmd->SetScissor(m_Window->GetSize());

				PushConstant pc = { m_EditorCamera.GetViewProjection(), m_VertexBuffer->GetHandle(), m_Image->GetHandle(), m_Sampler->GetHandle() };
				cmd->PushConstant(pc);

				cmd->BindIndexBuffer(m_IndexBuffer);
				cmd->DrawIndexed(6, 1, 0, 0, 0);
				cmd->EndRendering();

				ImGuiLayer::DrawFrame(cmd, m_Swapchain->GetCurrent());

				cmd->Present(m_Swapchain);
				m_Queue->Submit(cmd, m_Fence, m_Fence);
				m_Queue->Present(m_Swapchain, m_Fence);
			}

			m_Window->OnUpdate();
			m_EditorCamera.OnUpdate();

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
		m_Swapchain->Resize(e.GetWidth(), e.GetHeight());

		return true;
	}

}