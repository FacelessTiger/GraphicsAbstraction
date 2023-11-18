#include "Application.h"

#include <GraphicsAbstraction/Core/Core.h>

namespace GraphicsAbstraction {

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Color;
	};

	std::vector<Vertex> vertices;

	Application::Application()
	{
		GA_PROFILE_SCOPE();

		vertices.resize(3);
		vertices[0].Position = { 1.0f, 1.0f, 0.0f };
		vertices[1].Position = { -1.0f, 1.0f, 0.0f };
		vertices[2].Position = { 0.0f, -1.0f, 0.0f };

		vertices[0].Color = { 0.0f, 1.0f, 0.0f };
		vertices[1].Color = { 1.0f, 0.0f, 0.0f };
		vertices[2].Color = { 0.0f, 1.0f, 0.0f };

		m_Context = GraphicsContext::Create();
		m_Window = Window::Create();
		m_Window->SetEventCallback(GA_BIND_EVENT_FN(Application::OnEvent));

		m_Swapchain = Swapchain::Create(m_Window, m_Context);
		m_CommandPool = CommandPool::Create(m_Context, QueueType::Graphics);
		m_CommandBuffer = m_CommandPool->CreateCommandBuffer();

		Renderpass::Attachment colorAttachment;
		colorAttachment.LoadOperation = Renderpass::LoadOperation::Clear;
		colorAttachment.StoreOperation = Renderpass::StoreOperation::Store;
		colorAttachment.FinalImageLayout = Renderpass::ImageLayout::PresentSource;

		Renderpass::AttachmentReference colorAttachmentRef;
		colorAttachmentRef.AttachmentIndex = 0;
		colorAttachmentRef.ImageLayout = Renderpass::ImageLayout::ColorAttachmentOptimal;

		Renderpass::SubpassSpecification subpass;
		subpass.Bindpoint = Renderpass::PipelineBindpoint::Graphics;
		subpass.ColorAttachments = { colorAttachmentRef };

		Renderpass::Specification renderpassSpec;
		renderpassSpec.Attachments = { colorAttachment };
		renderpassSpec.Subpasses = { subpass };

		m_Renderpass = Renderpass::Create(renderpassSpec, m_Context, m_Swapchain);
		m_Fence = Fence::Create(m_Context);

		m_VertexBuffer = VertexBuffer::Create(m_Context, (uint32_t)(vertices.size() * sizeof(Vertex)));
		m_VertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position"	},
			{ ShaderDataType::Float3, "a_Color"		}
		});
		m_VertexBuffer->SetData(vertices.data(), (uint32_t)(vertices.size() * sizeof(Vertex)));

		m_QuadShader = Shader::Create(m_Context, "Assets/shaders/quad.glsl");

		Pipeline::Specification pipelineSpec;
		pipelineSpec.Renderpass = m_Renderpass;
		pipelineSpec.Shaders = { m_QuadShader };
		pipelineSpec.VertexBuffers = { m_VertexBuffer };
		pipelineSpec.Extent = { m_Window->GetWidth(), m_Window->GetHeight() };
		m_QuadPipeline = Pipeline::Create(m_Context, pipelineSpec);

		m_GPUProfilerContext = GA_GPU_PROFILER_CONTEXT(m_Context, m_CommandBuffer);
	}

	Application::~Application()
	{
		GA_PROFILE_SCOPE();

		GA_GPU_PROFILER_DESTROY(m_GPUProfilerContext);
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(GA_BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(GA_BIND_EVENT_FN(Application::OnWindowResize));
		dispatcher.Dispatch<KeyPressedEvent>(GA_BIND_EVENT_FN(Application::OnKeyPressed));
	}

	void Application::Run()
	{
		while (m_Running)
		{
			GA_PROFILE_SCOPE();
			m_Fence->Wait();

			uint32_t swapchainImageIndex = m_Swapchain->AcquireNextImage();
			m_CommandBuffer->Reset();
			m_CommandBuffer->Begin();

			{
				GA_PROFILE_GPU_SCOPE(m_GPUProfilerContext, m_CommandBuffer, "render");
				GA_PROFILE_GPU_COLLECT(m_GPUProfilerContext, m_CommandBuffer);

				glm::vec4 clearColor(0.0f, 0.0f, (float)abs(sin(m_FrameNumber / 120.0f)), 1.0f);
				m_Renderpass->Begin(m_Swapchain, m_CommandBuffer, clearColor, swapchainImageIndex);
				
				m_QuadPipeline->Bind(m_CommandBuffer, Renderpass::PipelineBindpoint::Graphics);
				m_VertexBuffer->Bind(m_CommandBuffer);

				m_CommandBuffer->Draw(3, 1);

				m_Renderpass->End(m_CommandBuffer);
			}
			m_CommandBuffer->End();

			m_CommandBuffer->Submit(m_Swapchain, m_Fence);
			m_CommandBuffer->Present(m_Swapchain, swapchainImageIndex);

			m_Window->OnUpdate();
			m_FrameNumber++;

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
		m_Swapchain->Resize(e.GetWidth(), e.GetHeight());
		m_Renderpass->Recreate(m_Swapchain);

		Pipeline::Specification pipelineSpec;
		pipelineSpec.Renderpass = m_Renderpass;
		pipelineSpec.Shaders = { m_QuadShader };
		pipelineSpec.VertexBuffers = { m_VertexBuffer };
		pipelineSpec.Extent = { m_Window->GetWidth(), m_Window->GetHeight() };
		m_QuadPipeline = Pipeline::Create(m_Context, pipelineSpec);

		return true;
	}

	bool Application::OnKeyPressed(KeyPressedEvent& e)
	{
		return true;
	}

}