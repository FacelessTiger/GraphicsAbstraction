#pragma once

#include <memory>

namespace GraphicsAbstraction {

	class Window;
	class GraphicsContext;
	class CommandBuffer;
	class Fence;

	class Swapchain
	{
	public:
		virtual ~Swapchain() = default;

		virtual uint32_t AcquireNextImage() const = 0;
		virtual void SubmitCommandBuffer(std::shared_ptr<CommandBuffer> cmd, std::shared_ptr<Fence> fence) const = 0;
		virtual void Present(uint32_t swapchainImageIndex) const = 0;

		static std::shared_ptr<Swapchain> Create(std::shared_ptr<Window> window, std::shared_ptr<GraphicsContext> context);
	};

}