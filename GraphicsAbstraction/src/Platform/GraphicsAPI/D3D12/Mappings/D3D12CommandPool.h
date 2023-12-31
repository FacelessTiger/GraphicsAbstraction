#pragma once

#include <GraphicsAbstraction/Renderer/CommandPool.h>
#include <GraphicsAbstraction/Renderer/CommandBuffer.h>
#include <GraphicsAbstraction/Renderer/Queue.h>

namespace GraphicsAbstraction {

	class D3D12CommandPool : public CommandPool
	{
	public:
		D3D12CommandPool(const Ref<Queue>& queue) { }

		void Reset() override { }
		Ref<CommandBuffer> Begin() const override { return nullptr; }
	};

}