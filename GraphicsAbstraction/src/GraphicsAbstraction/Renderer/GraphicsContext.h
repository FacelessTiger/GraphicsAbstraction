#pragma once

#include <GraphicsAbstraction/Renderer/Swapchain.h>

#include <memory>

namespace GraphicsAbstraction {

	class Window;

	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;

		static std::shared_ptr<GraphicsContext> Create();
	};
}