#pragma once

#include <memory>

#include <GraphicsAbstraction/Core/Log.h>
#include <GraphicsAbstraction/Core/Core.h>

namespace GraphicsAbstraction {

	class Window;
	class GraphicsContext;

	class Surface : public RefCounted
	{
	public:
		virtual ~Surface() = default;

		static Ref<Surface> Create(const Ref<Window>& window);
	};

}