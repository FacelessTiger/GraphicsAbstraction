#pragma once

#include <memory>

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
		static void Init(const std::shared_ptr<CommandPool>& commandPool, const std::shared_ptr<Swapchain>& swapchain, const std::shared_ptr<Window>& window, const std::shared_ptr<Queue>& queue, const std::shared_ptr<Fence>& fence);
		static void Shutdown();

		static void BeginFrame();
		static void DrawFrame(const std::shared_ptr<CommandBuffer>& cmd, const std::shared_ptr<Image>& image);
	private:
		static void SetDarkThemeColors();
		static void CreateFontTexture(const std::shared_ptr<CommandPool>& commandPool, const std::shared_ptr<Queue>& queue, const std::shared_ptr<Fence>& fence);
	};

}