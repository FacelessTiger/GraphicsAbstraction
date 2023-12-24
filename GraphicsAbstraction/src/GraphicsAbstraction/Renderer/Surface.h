#pragma once

#include <memory>

#include <GraphicsAbstraction/Core/Log.h>

namespace GraphicsAbstraction {

	class Window;
	class GraphicsContext;

	class Surface
	{
	public:
		virtual ~Surface() = default;

		static std::shared_ptr<Surface> Create(const std::shared_ptr<Window>& window);
	};

}