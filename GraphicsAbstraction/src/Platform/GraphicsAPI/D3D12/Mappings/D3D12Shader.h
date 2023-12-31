#pragma once

#include <GraphicsAbstraction/Renderer/Shader.h>

namespace GraphicsAbstraction {

	class D3D12Shader : public Shader
	{
	public:
		D3D12Shader(const std::string& path, ShaderStage stage) { }
	};

}