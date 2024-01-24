#pragma once

#include <GraphicsAbstraction/Core/Core.h>
#include <GraphicsAbstraction/Renderer/Queue.h>

#include <memory>

namespace GraphicsAbstraction {

	struct Window;

	enum class ShaderCompiledType
	{
		Spirv,
		Dxil
	};

	struct GA_DLL_LINK GraphicsContext
	{
		static void Init(uint32_t frameInFlightCount);

		static Ref<Queue> GetQueue(QueueType type);
		static ShaderCompiledType GetShaderCompiledType();
		static void SetFrameInFlight(uint32_t fif);
	};
}