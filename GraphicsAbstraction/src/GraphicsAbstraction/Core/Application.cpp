#include "Application.h"

#include <GraphicsAbstraction/Core/Core.h>

namespace GraphicsAbstraction {

	Application::Application()
	{
		GA_PROFILE_SCOPE();

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

		m_TriangleShader = Shader::Create(m_Context, "Assets/shaders/coloredTriangle.glsl");
		m_TrianglePipeline = Pipeline::Create(m_Context, m_TriangleShader, m_Renderpass, m_Swapchain->GetWidth(), m_Swapchain->GetHeight());

		m_GPUProfilerContext = GA_GPU_PROFILER_CONTEXT(m_Context, m_CommandBuffer);
	}

	Application::~Application()
	{
		GA_GPU_PROFILER_DESTROY(m_GPUProfilerContext);
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(GA_BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(GA_BIND_EVENT_FN(Application::OnWindowResize));
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

				Vector4 clearColor(0.0f, 0.0f, (float)abs(sin(m_FrameNumber / 120.0f)), 1.0f);
				m_Renderpass->Begin(m_Swapchain, m_CommandBuffer, clearColor, swapchainImageIndex);

				m_TrianglePipeline->Bind(m_CommandBuffer, Renderpass::PipelineBindpoint::Graphics);
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

		return true;
	}

}