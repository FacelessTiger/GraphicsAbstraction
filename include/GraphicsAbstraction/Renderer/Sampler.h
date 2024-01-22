#pragma once

#include <memory>
#include <GraphicsAbstraction/Core/Core.h>

namespace GraphicsAbstraction {

	enum class Filter
	{
		Nearest,
		Linear
	};

	struct GA_DLL_LINK Sampler : public RefCounted
	{
		uint32_t GetHandle() const;

		GA_RHI_TEMPLATE(Sampler, Filter min, Filter mag);
	};

}