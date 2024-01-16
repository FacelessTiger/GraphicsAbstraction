#pragma once

#include <memory>

#include <GraphicsAbstraction/Core/Log.h>
#include <GraphicsAbstraction/Core/Core.h>

namespace GraphicsAbstraction {

	class GraphicsContext;
	class CommandList;
	class Queue;

	class CommandAllocator : public RefCounted
	{
	public:
		virtual ~CommandAllocator() = default;

		virtual CommandAllocator* Reset() = 0;
		virtual Ref<CommandList> Begin() const = 0;

		static Ref<CommandAllocator> Create(const Ref<Queue>& queue);
	};

}