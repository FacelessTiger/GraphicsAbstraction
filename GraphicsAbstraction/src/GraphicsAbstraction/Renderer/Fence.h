#pragma once

#include <GraphicsAbstraction/Core/Core.h>

#include <memory>

namespace GraphicsAbstraction {

	class GraphicsContext;

	class Fence
	{
	public:
		virtual ~Fence() = default;

		virtual void Wait() const = 0;

		static std::shared_ptr<Fence> Create(std::shared_ptr<GraphicsContext> context);
	};

}