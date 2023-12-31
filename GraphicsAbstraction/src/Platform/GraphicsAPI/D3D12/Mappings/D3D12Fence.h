#pragma once

#include <GraphicsAbstraction/Renderer/Fence.h>

namespace GraphicsAbstraction {

	class D3D12Fence : public Fence
	{
	public:
		D3D12Fence() { }

		void Wait() override { }
	};

}