#pragma once

#include <memory>

#include <GraphicsAbstraction/Core/Log.h>

namespace GraphicsAbstraction {

	class Fence
	{
	public:
		virtual ~Fence() = default;

		virtual void Wait() = 0;

		static std::shared_ptr<Fence> Create();
	};

}