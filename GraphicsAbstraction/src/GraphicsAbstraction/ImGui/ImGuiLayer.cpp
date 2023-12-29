#include "ImGuiLayer.h"

#include <array>
#include <vulkan/vulkan.h>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

#include <GraphicsAbstraction/Renderer/CommandPool.h>
#include <GraphicsAbstraction/Renderer/CommandBuffer.h>
#include <GraphicsAbstraction/Renderer/Fence.h>
#include <GraphicsAbstraction/Renderer/Buffer.h>
#include <GraphicsAbstraction/Renderer/Sampler.h>
#include <GraphicsAbstraction/Renderer/Shader.h>
#include <GraphicsAbstraction/Renderer/Queue.h>
#include <GraphicsAbstraction/Core/Window.h>

namespace GraphicsAbstraction {

	struct ImGuiLayerData
	{
		Ref<Shader> VertexShader;
		Ref<Shader> PixelShader;
		Ref<Sampler> Sampler;
		Ref<Buffer> IndexBuffer, VertexBuffer;
		Ref<Image> FontImage;
	};

	struct PushConstant
	{
		glm::vec2 scale;
		glm::vec2 offset;
		uint32_t vertices;
		uint32_t texture;
		uint32_t sampler;
	};

	static ImGuiLayerData* s_Data;

	void ImGuiLayer::Init(Ref<CommandPool>& commandPool, Ref<Swapchain>& swapchain, Ref<Window>& window, Ref<Queue>& queue, Ref<Fence>& fence)
	{
		s_Data = new ImGuiLayerData();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		SetDarkThemeColors();

		GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(window->GetNativeWindow());
		ImGui_ImplGlfw_InitForVulkan(glfwWindow, true);

		// do init
		GA_CORE_ASSERT(!io.BackendRendererUserData, "Imgui already has a backend!");
		io.BackendRendererUserData = s_Data;
		io.BackendRendererName = "GraphicsAbstraction";
		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
		io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;  // We can create multi-viewports on the Renderer side (optional)

		s_Data->VertexShader = Shader::Create("Assets/shaders/imguiVertex.hlsl", ShaderStage::Vertex);
		s_Data->PixelShader = Shader::Create("Assets/shaders/imguiPixel.hlsl", ShaderStage::Pixel);
		s_Data->Sampler = Sampler::Create(Filter::Linear, Filter::Linear);

		s_Data->IndexBuffer = Buffer::Create(sizeof(uint32_t), BufferUsage::IndexBuffer, BufferFlags::Mapped);
		s_Data->VertexBuffer = Buffer::Create(sizeof(ImDrawVert), BufferUsage::StorageBuffer, BufferFlags::Mapped);

		CreateFontTexture(commandPool, queue, fence);
	}

	void ImGuiLayer::Shutdown()
	{
		ImGui_ImplGlfw_Shutdown();
		delete s_Data;
	}

	void ImGuiLayer::BeginFrame()
	{
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::DrawFrame(Ref<CommandBuffer>& cmd, Ref<Image> image)
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGui::Render();
		ImDrawData* drawData = ImGui::GetDrawData();

		int framebufferWidth = (int)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
		int framebufferHeight = (int)(drawData->DisplaySize.y * drawData->FramebufferScale.y);
		if (framebufferWidth <= 0 || framebufferHeight <= 0) return;

		if (drawData->TotalVtxCount > 0)
		{
			uint32_t indexSize = drawData->TotalIdxCount * sizeof(ImDrawIdx);
			uint32_t vertexSize = drawData->TotalVtxCount * (sizeof(ImDrawVert) + 12);
			if (s_Data->IndexBuffer->GetSize() < indexSize) s_Data->IndexBuffer = Buffer::Create(indexSize, BufferUsage::IndexBuffer, BufferFlags::Mapped);
			if (s_Data->VertexBuffer->GetSize() < vertexSize) s_Data->VertexBuffer = Buffer::Create(vertexSize, BufferUsage::StorageBuffer, BufferFlags::Mapped);

			uint32_t indexOffset = 0;
			uint32_t vertexOffset = 0;
			for (int i = 0; i < drawData->CmdListsCount; i++)
			{
				const ImDrawList* drawList = drawData->CmdLists[i];
				uint32_t indexByteSize = drawList->IdxBuffer.Size * sizeof(ImDrawIdx);
				uint32_t vertexByteSize = drawList->VtxBuffer.Size * sizeof(ImDrawVert);

				s_Data->IndexBuffer->SetData(drawList->IdxBuffer.Data, indexByteSize, indexOffset);
				s_Data->VertexBuffer->SetData(drawList->VtxBuffer.Data, vertexByteSize, vertexOffset);

				indexOffset += indexByteSize;
				vertexOffset += vertexByteSize;
			}
		}

		// Will project scissor/clipping rectangles into framebuffer space
		glm::vec2 clipOffset = { drawData->DisplayPos.x, drawData->DisplayPos.y };         // (0,0) unless using multi-viewports
		glm::vec2 clipScale = { drawData->FramebufferScale.x, drawData->FramebufferScale.y }; // (1,1) unless using retina display which are often (2,2)

		cmd->BeginRendering({ framebufferWidth, framebufferHeight }, { image });
		cmd->BindShaders({ s_Data->VertexShader, s_Data->PixelShader });
		cmd->EnableColorBlend(Blend::SrcAlpha, Blend::OneMinusSrcAlpha, BlendOp::Add, Blend::One, Blend::Zero, BlendOp::Add);
		cmd->SetViewport({ framebufferWidth, framebufferHeight });
		cmd->BindIndexBuffer(s_Data->IndexBuffer);

		PushConstant pc;
		pc.scale = { 2.0f / drawData->DisplaySize.x, 2.0f / drawData->DisplaySize.y };
		pc.offset = { -1.0f - drawData->DisplayPos.x * pc.scale.x, -1.0f - drawData->DisplayPos.y * pc.scale.y };
		pc.vertices = s_Data->VertexBuffer->GetHandle();
		pc.sampler = s_Data->Sampler->GetHandle();

		int vertexOffset = 0;
		int indexOffset = 0;
		for (int i = 0; i < drawData->CmdListsCount; i++)
		{
			const ImDrawList* drawList = drawData->CmdLists[i];
			for (int j = 0; j < drawList->CmdBuffer.Size; j++)
			{
				const ImDrawCmd& drawCmd = drawList->CmdBuffer[j];

				glm::vec2 clipMin = { (drawCmd.ClipRect.x - clipOffset.x) * clipScale.x, (drawCmd.ClipRect.y - clipOffset.y) * clipScale.y };
				glm::vec2 clipMax = { (drawCmd.ClipRect.z - clipOffset.x) * clipScale.x, (drawCmd.ClipRect.w - clipOffset.y) * clipScale.y };

				// Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
				if (clipMin.x < 0.0f) { clipMin.x = 0.0f; }
				if (clipMin.y < 0.0f) { clipMin.y = 0.0f; }
				if (clipMax.x > framebufferWidth) { clipMax.x = (float)framebufferWidth; }
				if (clipMax.y > framebufferHeight) { clipMax.y = (float)framebufferHeight; }
				if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y) continue;

				pc.texture = (uint32_t)drawCmd.TextureId;
				cmd->PushConstant(pc);

				cmd->SetScissor({ clipMax.x - clipMin.x, clipMax.y - clipMin.y }, { clipMin.x, clipMin.y });
				cmd->DrawIndexed(drawCmd.ElemCount, 1, drawCmd.IdxOffset + indexOffset, drawCmd.VtxOffset + vertexOffset, 0);
			}

			vertexOffset += drawList->VtxBuffer.Size;
			indexOffset += drawList->IdxBuffer.Size;
		}
		cmd->EndRendering();

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void ImGuiLayer::SetDarkThemeColors()
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = { 0.1f, 0.105f, 0.11f, 1.0f };

		// Headers
		colors[ImGuiCol_Header] = { 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = { 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = { 0.15f, 0.1505f, 0.151f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button] = { 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = { 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_ButtonActive] = { 0.15f, 0.1505f, 0.151f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = { 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = { 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = { 0.15f, 0.1505f, 0.151f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab] = { 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabHovered] = { 0.38f, 0.3805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabActive] = { 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = { 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = { 0.2f, 0.205f, 0.21f, 1.0f };

		// Title
		colors[ImGuiCol_TitleBg] = { 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = { 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = { 0.15f, 0.1505f, 0.151f, 1.0f };
	}

	void ImGuiLayer::CreateFontTexture(Ref<CommandPool>& commandPool, Ref<Queue>& queue, Ref<Fence>& fence)
	{
		ImGuiIO& io = ImGui::GetIO();

		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

		s_Data->FontImage = Image::Create({ width, height }, ImageFormat::R8G8B8A8_UNORM, ImageUsage::Sampled | ImageUsage::TransferDst);
		auto buffer = Buffer::Create(width * height * 4, BufferUsage::TransferSrc, BufferFlags::Mapped);
		buffer->SetData(pixels);

		commandPool->Reset();
		auto cmd = commandPool->Begin();
		cmd->CopyToImage(buffer, s_Data->FontImage);
		queue->Submit(cmd, nullptr, fence);
		fence->Wait();

		io.Fonts->SetTexID((ImTextureID)s_Data->FontImage->GetHandle());
	}

}