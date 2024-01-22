#pragma once

#include <memory>
#include <GraphicsAbstraction/Core/Core.h>

namespace GraphicsAbstraction {

	struct GraphicsContext;
	struct CommandList;
	struct Queue;

	struct GA_DLL_LINK CommandAllocator : public RefCounted
	{
		CommandAllocator* Reset();
		Ref<CommandList> Begin() const;

		GA_RHI_TEMPLATE(CommandAllocator, const Ref<Queue>& queue);
	};

}