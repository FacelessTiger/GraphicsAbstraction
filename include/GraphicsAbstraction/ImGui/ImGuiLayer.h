#pragma once

#include <memory>
#include <GraphicsAbstraction/Renderer/Image.h>

namespace GraphicsAbstraction {

	struct CommandAllocator;
	struct CommandList;
	struct Swapchain;
	struct Window;
	struct Queue;
	struct Fence;
	struct Image;
	class Event;

	class ImGuiLayer
	{
	public:
		static void Init(Ref<CommandAllocator>& commandPool, Ref<Swapchain>& swapchain, Ref<Window>& window, Ref<Queue>& queue, Ref<Fence>& fence);
		static void Shutdown();

		static void BeginFrame();
		static void DrawFrame(Ref<CommandList>& cmd, Ref<Image> image);

		static void OnEvent(Event& e);
	private:
		static void SetDarkThemeColors();
		static void CreateFontTexture(Ref<CommandAllocator>& commandPool, Ref<Queue>& queue, Ref<Fence>& fence);
	};

}