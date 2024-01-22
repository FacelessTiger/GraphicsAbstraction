#pragma once

#include <memory>
#include <string>

#include <GraphicsAbstraction/Core/Core.h>

namespace GraphicsAbstraction {

	enum class ShaderStage
	{
		Vertex,
		Pixel,
		Compute
	};

	struct GA_DLL_LINK Shader : public RefCounted
	{
		GA_RHI_TEMPLATE(Shader, const std::string& path, ShaderStage stage);
	};

}