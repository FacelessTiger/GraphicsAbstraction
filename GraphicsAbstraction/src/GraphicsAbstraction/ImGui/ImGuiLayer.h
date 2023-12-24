#pragma once

#include <memory>

namespace GraphicsAbstraction {

	class GraphicsContext;
	class CommandPool;
	class VulkanContext;
	class CommandBuffer;
	class Swapchain;
	class Window;
	class Image;

	class ImGuiLayer
	{
	public:
		static void Init(const std::shared_ptr<CommandPool>& commandPool, const std::shared_ptr<Swapchain>& swapchain, const std::shared_ptr<Window>& window);
		static void Shutdown();

		static void BeginFrame();
		static void DrawFrame(const std::shared_ptr<CommandBuffer>& cmd, const std::shared_ptr<Image>& image);
	private:
		static void SetDarkThemeColors();
	};

}