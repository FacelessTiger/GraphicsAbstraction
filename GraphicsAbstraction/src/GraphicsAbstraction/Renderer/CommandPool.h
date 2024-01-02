#pragma once

#include <memory>

#include <GraphicsAbstraction/Core/Log.h>
#include <GraphicsAbstraction/Core/Core.h>

namespace GraphicsAbstraction {

	class GraphicsContext;
	class CommandBuffer;
	class Queue;

	class CommandPool : public RefCounted
	{
	public:
		virtual ~CommandPool() = default;

		virtual CommandPool* Reset() = 0;
		virtual Ref<CommandBuffer> Begin() const = 0;

		static Ref<CommandPool> Create(const Ref<Queue>& queue);
	};

}