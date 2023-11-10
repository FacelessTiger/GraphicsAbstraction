#pragma once

#include <memory>

namespace GraphicsAbstraction {

	class Window;
	class GraphicsContext;

	class Swapchain
	{
	public:
		virtual ~Swapchain() = default;

		static std::shared_ptr<Swapchain> Create(std::shared_ptr<Window> window, std::shared_ptr<GraphicsContext> context);
	};

}