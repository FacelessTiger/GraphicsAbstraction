#pragma once

#include <memory>
#include <GraphicsAbstraction/Renderer/Image.h>

namespace GraphicsAbstraction {

	class CommandPool;
	class CommandBuffer;
	class Swapchain;
	class Window;
	class Queue;
	class Fence;
	class Image;

	class ImGuiLayer
	{
	public:
		static void Init(Ref<CommandPool>& commandPool, Ref<Swapchain>& swapchain, Ref<Window>& window, Ref<Queue>& queue, Ref<Fence>& fence);
		static void Shutdown();

		static void BeginFrame();
		static void DrawFrame(Ref<CommandBuffer>& cmd, Ref<Image> image);
	private:
		static void SetDarkThemeColors();
		static void CreateFontTexture(Ref<CommandPool>& commandPool, Ref<Queue>& queue, Ref<Fence>& fence);
	};

}