#pragma once

#include <memory>

namespace GraphicsAbstraction {

	class Swapchain;
	class Fence;

	class CommandBuffer
	{
	public:
		virtual ~CommandBuffer() = default;

		virtual void Reset() const = 0;
		virtual void Submit(std::shared_ptr<Swapchain> swapchain, std::shared_ptr<Fence> fence) const = 0;
		virtual void Draw(uint32_t vertexCount, uint32_t instanceCount) const = 0;;
		virtual void Present(std::shared_ptr<Swapchain> swapchain, uint32_t swapchainImageIndex) const = 0;

		virtual void Begin() const = 0;
		virtual void End() const = 0;
	};

}