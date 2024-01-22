#pragma once

#include <memory>
#include <GraphicsAbstraction/Core/Core.h>

namespace GraphicsAbstraction {

	struct GA_DLL_LINK Fence : public RefCounted
	{
		void Wait() const;

		GA_RHI_TEMPLATE(Fence)
	};

}