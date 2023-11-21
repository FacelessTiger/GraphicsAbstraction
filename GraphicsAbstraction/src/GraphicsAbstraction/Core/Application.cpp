#include "Application.h"

#include <GraphicsAbstraction/Core/Core.h>
#include <GraphicsAbstraction/Renderer/Image.h>

#include <glm/gtx//transform.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace GraphicsAbstraction {

	Application* Application::s_Instance = nullptr;

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
	};

	struct MeshPush
	{
		glm::mat4 ViewProjection;
		glm::mat4 Model;
		glm::vec3 ViewPos;
	};

	std::vector<Vertex> vertices;

	void LoadFromObj(const char* filename)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		std::string warn;
		std::string err;

		tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, nullptr);
		
		if (!warn.empty()) GA_CORE_WARN(warn);
		GA_CORE_ASSERT(err.empty(), err);

		for (int s = 0; s < shapes.size(); s++)
		{
			size_t indexOffset = 0;
			for (int f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
			{
				int fv = 3;
				for (int v = 0; v < fv; v++)
				{
					tinyobj::index_t idx = shapes[s].mesh.indices[indexOffset + v];

					tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
					tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
					tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
					
					tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
					tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
					tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];

					Vertex vertex;
					vertex.Position.x = vx;
					vertex.Position.y = vy;
					vertex.Position.z = vz;
					
					vertex.Normal.x = nx;
					vertex.Normal.y = ny;
					vertex.Normal.z = nz;

					vertices.push_back(vertex);
				}

				indexOffset += fv;
			}
		}
	}

	Application::Application()
	{
		GA_PROFILE_SCOPE();

		GA_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		LoadFromObj("Assets/models/monkey_smooth.obj");

		//std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

		m_Context = GraphicsContext::Create();
		m_Window = Window::Create();
		m_Window->SetEventCallback(GA_BIND_EVENT_FN(Application::OnEvent));

		m_Swapchain = Swapchain::Create(m_Window, m_Context);
		m_CommandPool = CommandPool::Create(m_Context, QueueType::Graphics);
		m_CommandBuffer = m_CommandPool->CreateCommandBuffer();

		std::vector<std::shared_ptr<Image>> swapchainImages = m_Swapchain->GetImages(); // image holds both the image and image view and samples format and whatever other metadata
		
		Image::Specification depthImageSpec;
		depthImageSpec.Format = Image::Format::D32SFloat;
		depthImageSpec.Size = m_Swapchain->GetSize();
		std::shared_ptr<Image> depthImage = Image::Create(m_Context, depthImageSpec);

		m_RenderpassSpec.ColorOutputs = { swapchainImages };
		m_RenderpassSpec.DepthStencilOutput = { depthImage };
		m_RenderpassSpec.FramebufferCount = (uint32_t)swapchainImages.size();
		m_RenderpassSpec.Size = m_Swapchain->GetSize();

		m_Renderpass = Renderpass::Create(m_Context, m_RenderpassSpec);
		m_Fence = Fence::Create(m_Context);

		m_VertexBuffer = VertexBuffer::Create(m_Context, (uint32_t)(vertices.size() * sizeof(Vertex)));
		m_VertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position"	},
			{ ShaderDataType::Float3, "a_Normal"	}
		});
		m_VertexBuffer->SetData(vertices.data(), (uint32_t)(vertices.size() * sizeof(Vertex)));

		//m_IndexBuffer = IndexBuffer::Create(m_Context, indices.data(), (uint32_t)indices.size());
		m_QuadShader = Shader::Create(m_Context, "Assets/shaders/quad.glsl");

		m_PushConstant = PushConstant::Create(0, sizeof(MeshPush), PushConstant::ShaderStage::Vertex);

		Pipeline::Specification pipelineSpec;
		pipelineSpec.Renderpass = m_Renderpass;
		pipelineSpec.Shaders = { m_QuadShader };
		pipelineSpec.VertexBuffers = { m_VertexBuffer };
		pipelineSpec.PushConstants = { m_PushConstant };
		pipelineSpec.Extent = { m_Window->GetWidth(), m_Window->GetHeight() };
		m_QuadPipeline = Pipeline::Create(m_Context, pipelineSpec);

		m_GPUProfilerContext = GA_GPU_PROFILER_CONTEXT(m_Context, m_CommandBuffer);
		m_EditorCamera = EditorCamera(30.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
	}

	Application::~Application()
	{
		GA_PROFILE_SCOPE();

		GA_GPU_PROFILER_DESTROY(m_GPUProfilerContext);
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
			m_Fence->Wait();

			uint32_t swapchainImageIndex = m_Swapchain->AcquireNextImage();
			m_CommandBuffer->Reset();
			m_CommandBuffer->Begin();

			{
				GA_PROFILE_GPU_SCOPE(m_GPUProfilerContext, m_CommandBuffer, "render");
				GA_PROFILE_GPU_COLLECT(m_GPUProfilerContext, m_CommandBuffer);

				glm::vec4 clearColor(0.0f, 0.0f, (float)abs(sin(m_FrameNumber / 120.0f)), 1.0f);
				ClearValue colorClear(clearColor);
				ClearValue depthClear(1.0f, 0);

				m_Renderpass->Begin(m_Swapchain->GetSize(), m_CommandBuffer, { colorClear, depthClear }, swapchainImageIndex);
				
				m_QuadPipeline->Bind(m_CommandBuffer);
				m_VertexBuffer->Bind(m_CommandBuffer);
				//m_IndexBuffer->Bind(m_CommandBuffer);

				glm::mat4 model = glm::rotate(glm::mat4{ 1.0f }, glm::radians(m_FrameNumber * 0.4f), glm::vec3(0, 1, 0));

				MeshPush meshPush;
				meshPush.ViewProjection = m_EditorCamera.GetViewProjection();
				meshPush.Model = model;
				meshPush.ViewPos = m_EditorCamera.GetPosition();

				m_PushConstant->Push(m_CommandBuffer, m_QuadPipeline, &meshPush);
				//m_CommandBuffer->DrawIndexed(m_IndexBuffer->GetCount(), 1);
				m_CommandBuffer->Draw((uint32_t)vertices.size(), 1);

				m_Renderpass->End(m_CommandBuffer);
			}
			m_CommandBuffer->End();

			m_CommandBuffer->Submit(m_Swapchain, m_Fence);
			m_CommandBuffer->Present(m_Swapchain, swapchainImageIndex);

			m_Window->OnUpdate();
			m_EditorCamera.OnUpdate();
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

		Image::Specification depthImageSpec;
		depthImageSpec.Format = Image::Format::D32SFloat;
		depthImageSpec.Size = m_Swapchain->GetSize();
		std::shared_ptr<Image> depthImage = Image::Create(m_Context, depthImageSpec);

		m_RenderpassSpec.Size = { m_Window->GetWidth(), m_Window->GetHeight() };
		m_RenderpassSpec.ColorOutputs = { m_Swapchain->GetImages() };
		m_RenderpassSpec.DepthStencilOutput = { depthImage };
		m_Renderpass->Recreate(m_RenderpassSpec);

		Pipeline::Specification pipelineSpec;
		pipelineSpec.Renderpass = m_Renderpass;
		pipelineSpec.Shaders = { m_QuadShader };
		pipelineSpec.VertexBuffers = { m_VertexBuffer };
		pipelineSpec.PushConstants = { m_PushConstant };
		pipelineSpec.Extent = { m_Window->GetWidth(), m_Window->GetHeight() };
		m_QuadPipeline = Pipeline::Create(m_Context, pipelineSpec);

		m_EditorCamera.SetViewportSize((float)e.GetWidth(), (float)e.GetHeight());

		return true;
	}

}