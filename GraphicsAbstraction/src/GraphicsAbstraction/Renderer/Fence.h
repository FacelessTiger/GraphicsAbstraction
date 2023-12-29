#pragma once

#include <memory>

#include <GraphicsAbstraction/Core/Log.h>
#include <GraphicsAbstraction/Core/Core.h>

namespace GraphicsAbstraction {

	class Fence : public RefCounted
	{
	public:
		virtual ~Fence() = default;

		virtual void Wait() = 0;

		static Ref<Fence> Create();
	};

}