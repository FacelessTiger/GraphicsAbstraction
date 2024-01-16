#pragma once

#include <memory>
#include <GraphicsAbstraction/Renderer/Image.h>

namespace GraphicsAbstraction {

	class CommandAllocator;
	class CommandList;
	class Swapchain;
	class Window;
	class Queue;
	class Fence;
	class Event;
	class Image;

	class ImGuiLayer
	{
	public:
		static void Init(Ref<CommandAllocator>& commandPool, Ref<Swapchain>& swapchain, Ref<Window>& window, Ref<Queue>& queue, Ref<Fence>& fence);
		static void Shutdown();

		static void BeginFrame();
		static void DrawFrame(Ref<CommandList>& cmd, Ref<Image> image);
	private:
		static void SetDarkThemeColors();
		static void CreateFontTexture(Ref<CommandAllocator>& commandPool, Ref<Queue>& queue, Ref<Fence>& fence);
	};

}