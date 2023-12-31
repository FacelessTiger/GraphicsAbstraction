#pragma once

#include <GraphicsAbstraction/Renderer/Sampler.h>

namespace GraphicsAbstraction {

	class D3D12Sampler : public Sampler
	{
	public:
		D3D12Sampler(Filter min, Filter mag) { }

		uint32_t GetHandle() const override { return 0; }
	};

}