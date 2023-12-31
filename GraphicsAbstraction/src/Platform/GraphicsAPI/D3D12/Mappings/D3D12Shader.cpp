#include "D3D12Shader.h"

namespace GraphicsAbstraction {

	Ref<Shader> Shader::Create(const std::string& path, ShaderStage stage)
	{
		return CreateRef<D3D12Shader>(path, stage);
	}

}