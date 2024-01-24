#define GA_DLL_LINK
#include <GraphicsAbstraction/ImGui/ImGuiLayer.h>

#include <iostream>

#include <array>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <GraphicsAbstraction/GraphicsAbstraction.h>

namespace GraphicsAbstraction {

	namespace Utils {

		ImGuiKey GAKeyToImGui(KeyCode code)
		{
			switch (code)
			{
				case Key::Space:		return ImGuiKey_Space;
				case Key::Apostrophe:	return ImGuiKey_Apostrophe;
				case Key::Comma:		return ImGuiKey_Comma;
				case Key::Minus:		return ImGuiKey_Minus;
				case Key::Period:		return ImGuiKey_Period;
				case Key::Slash:		return ImGuiKey_Slash;

				case Key::D0:			return ImGuiKey_0;
				case Key::D1:			return ImGuiKey_1;
				case Key::D2:			return ImGuiKey_2;
				case Key::D3:			return ImGuiKey_3;
				case Key::D4:			return ImGuiKey_4;
				case Key::D5:			return ImGuiKey_5;
				case Key::D6:			return ImGuiKey_6;
				case Key::D7:			return ImGuiKey_7;
				case Key::D8:			return ImGuiKey_8;
				case Key::D9:			return ImGuiKey_9;

				case Key::Semicolon:	return ImGuiKey_Semicolon;
				case Key::Equal:		return ImGuiKey_Equal;

				case Key::A:			return ImGuiKey_A;
				case Key::B:			return ImGuiKey_B;
				case Key::C:			return ImGuiKey_C;
				case Key::D:			return ImGuiKey_D;
				case Key::E:			return ImGuiKey_E;
				case Key::F:			return ImGuiKey_F;
				case Key::G:			return ImGuiKey_G;
				case Key::H:			return ImGuiKey_H;
				case Key::I:			return ImGuiKey_I;
				case Key::J:			return ImGuiKey_J;
				case Key::K:			return ImGuiKey_K;
				case Key::L:			return ImGuiKey_L;
				case Key::M:			return ImGuiKey_M;
				case Key::N:			return ImGuiKey_N;
				case Key::O:			return ImGuiKey_O;
				case Key::P:			return ImGuiKey_P;
				case Key::Q:			return ImGuiKey_Q;
				case Key::R:			return ImGuiKey_R;
				case Key::S:			return ImGuiKey_S;
				case Key::T:			return ImGuiKey_T;
				case Key::U:			return ImGuiKey_U;
				case Key::V:			return ImGuiKey_V;
				case Key::W:			return ImGuiKey_W;
				case Key::X:			return ImGuiKey_X;
				case Key::Y:			return ImGuiKey_Y;
				case Key::Z:			return ImGuiKey_Z;

				case Key::LeftBracket:	return ImGuiKey_LeftBracket;
				case Key::Backslash:	return ImGuiKey_Backslash;
				case Key::RightBracket:	return ImGuiKey_RightBracket;
				case Key::GraveAccent:	return ImGuiKey_GraveAccent;

				case Key::Escape:		return ImGuiKey_Escape;
				case Key::Enter:		return ImGuiKey_Enter;
				case Key::Tab:			return ImGuiKey_Tab;
				case Key::Backspace:	return ImGuiKey_Backspace;
				case Key::Insert:		return ImGuiKey_Insert;
				case Key::Delete:		return ImGuiKey_Delete;
				case Key::Right:		return ImGuiKey_RightArrow;
				case Key::Left:			return ImGuiKey_LeftArrow;
				case Key::Down:			return ImGuiKey_DownArrow;
				case Key::Up:			return ImGuiKey_UpArrow;
				case Key::PageDown:		return ImGuiKey_PageDown;
				case Key::PageUp:		return ImGuiKey_PageUp;
				case Key::Home:			return ImGuiKey_Home;
				case Key::End:			return ImGuiKey_End;
				case Key::CapsLock:		return ImGuiKey_CapsLock;
				case Key::ScrollLock:	return ImGuiKey_ScrollLock;
				case Key::NumLock:		return ImGuiKey_NumLock;
				case Key::PrintScreen:	return ImGuiKey_PrintScreen;
				case Key::Pause:		return ImGuiKey_Pause;
				case Key::F1:			return ImGuiKey_F1;
				case Key::F2:			return ImGuiKey_F2;
				case Key::F3:			return ImGuiKey_F3;
				case Key::F4:			return ImGuiKey_F4;
				case Key::F5:			return ImGuiKey_F5;
				case Key::F6:			return ImGuiKey_F6;
				case Key::F7:			return ImGuiKey_F7;
				case Key::F8:			return ImGuiKey_F8;
				case Key::F9:			return ImGuiKey_F9;
				case Key::F10:			return ImGuiKey_F10;
				case Key::F11:			return ImGuiKey_F11;
				case Key::F12:			return ImGuiKey_F12;

				case Key::KP0:			return ImGuiKey_Keypad0;
				case Key::KP1:			return ImGuiKey_Keypad1;
				case Key::KP2:			return ImGuiKey_Keypad2;
				case Key::KP3:			return ImGuiKey_Keypad3;
				case Key::KP4:			return ImGuiKey_Keypad4;
				case Key::KP5:			return ImGuiKey_Keypad5;
				case Key::KP6:			return ImGuiKey_Keypad6;
				case Key::KP7:			return ImGuiKey_Keypad7;
				case Key::KP8:			return ImGuiKey_Keypad8;
				case Key::KP9:			return ImGuiKey_Keypad9;
				case Key::KPDecimal:	return ImGuiKey_KeypadDecimal;
				case Key::KPDivide:		return ImGuiKey_KeypadDivide;
				case Key::KPMultiply:	return ImGuiKey_KeypadMultiply;
				case Key::KPSubtract:	return ImGuiKey_KeypadSubtract;
				case Key::KPAdd:		return ImGuiKey_KeypadAdd;
				case Key::KPEnter:		return ImGuiKey_KeyPadEnter;
				case Key::KPEqual:		return ImGuiKey_KeypadEqual;

				case Key::LeftShift:	return ImGuiKey_LeftShift;
				case Key::LeftControl:	return ImGuiKey_LeftCtrl;
				case Key::LeftAlt:		return ImGuiKey_LeftAlt;
				case Key::LeftSuper:	return ImGuiKey_LeftSuper;
				case Key::RightShift:	return ImGuiKey_RightShift;
				case Key::RightControl:	return ImGuiKey_RightCtrl;
				case Key::RightAlt:		return ImGuiKey_RightAlt;
				case Key::RightSuper:	return ImGuiKey_RightSuper;
				case Key::Menu:			return ImGuiKey_Menu;
			}

			return ImGuiKey_None;
		}

	}

	struct ImGuiLayerData
	{
		Ref<Shader> VertexShader;
		Ref<Shader> PixelShader;
		Ref<Sampler> Sampler;
		Ref<Buffer> IndexBuffer, VertexBuffer;
		Ref<Image> FontImage;

		Ref<Window> MainWindow;
	};

	struct PushConstant
	{
		glm::vec2 scale;
		glm::vec2 offset;
		uint32_t vertices;
		uint32_t texture;
		uint32_t sampler;
	};

#include "ImGuiShaders.inl"
	static ImGuiLayerData* s_Data;

	void ImGuiLayer::Init(Ref<CommandAllocator>& commandPool, Ref<Swapchain>& swapchain, Ref<Window>& window, Ref<Queue>& queue, Ref<Fence>& fence)
	{
		s_Data = new ImGuiLayerData();
		s_Data->MainWindow = window;

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

		// do init
		assert((!io.BackendRendererUserData || !io.BackendPlatformUserData) && "Imgui already has a backend!");
		io.BackendRendererUserData = s_Data;
		io.BackendRendererName = "GraphicsAbstraction Renderer";
		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
		//io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;  // We can create multi-viewports on the Renderer side (optional)

		std::vector<uint32_t> compiledVertex;
		//s_Data->VertexShader = Shader::Create("Assets/shaders/imguiVertex.hlsl", ShaderStage::Vertex, &compiledVertex);
		s_Data->VertexShader = Shader::Create((GraphicsContext::GetShaderCompiledType() == ShaderCompiledType::Spirv) ? s_VertexSpirv : s_VertexDxil, ShaderStage::Vertex);
		s_Data->PixelShader = Shader::Create((GraphicsContext::GetShaderCompiledType() == ShaderCompiledType::Spirv) ? s_PixelSpirv : s_PixelDxil, ShaderStage::Pixel);
		s_Data->Sampler = Sampler::Create(Filter::Linear, Filter::Linear);

		for (uint32_t data : compiledVertex)
			std::cout << std::hex << "0x" << data << ", ";

		s_Data->IndexBuffer = Buffer::Create(sizeof(uint32_t), BufferUsage::IndexBuffer, BufferFlags::Mapped);
		s_Data->VertexBuffer = Buffer::Create(sizeof(ImDrawVert), BufferUsage::StorageBuffer, BufferFlags::Mapped);

		CreateFontTexture(commandPool, queue, fence);
	}

	void ImGuiLayer::Shutdown()
	{
		delete s_Data;
	}

	void ImGuiLayer::BeginFrame()
	{
		auto size = s_Data->MainWindow->GetSize();
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = { size.x, size.y };

		ImGui::NewFrame();
	}

	void ImGuiLayer::DrawFrame(Ref<CommandList>& cmd, Ref<Image> image)
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
			uint32_t vertexSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
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
		cmd->SetFillMode(FillMode::Solid);
		cmd->DisableDepthTest();
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

				pc.texture = (uint32_t)(uint64_t)drawCmd.TextureId;
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

	void ImGuiLayer::OnEvent(Event& e)
	{
		ImGuiIO& io = ImGui::GetIO();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseMovedEvent>([&](MouseMovedEvent& e) {
			io.AddMousePosEvent(e.GetX(), e.GetY());
			return true;
		});

		dispatcher.Dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent& e) {
			io.AddMouseButtonEvent(e.GetMouseButton(), true);
			return true;
		});

		dispatcher.Dispatch<MouseButtonReleasedEvent>([&](MouseButtonReleasedEvent& e) {
			io.AddMouseButtonEvent(e.GetMouseButton(), false);
			return true;
		});

		dispatcher.Dispatch<KeyPressedEvent>([&](KeyPressedEvent& e) {
			io.AddKeyEvent(Utils::GAKeyToImGui(e.GetKeyCode()), true);
			return true;
		});

		dispatcher.Dispatch<KeyReleasedEvent>([&](KeyReleasedEvent& e) {
			io.AddKeyEvent(Utils::GAKeyToImGui(e.GetKeyCode()), false);
			return true;
		});

		dispatcher.Dispatch<KeyTypedEvent>([&](KeyTypedEvent& e) {
			io.AddInputCharacter(e.GetKey());
			return true;
		});
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

	void ImGuiLayer::CreateFontTexture(Ref<CommandAllocator>& commandPool, Ref<Queue>& queue, Ref<Fence>& fence)
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

		io.Fonts->SetTexID((ImTextureID)(uint64_t)s_Data->FontImage->GetSampledHandle());
	}

}