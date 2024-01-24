#pragma once

#include <memory>
#include <vector>
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
		ShaderStage GetStage() const;

		static Ref<Shader> Create(const std::vector<uint32_t>& data, ShaderStage stage);
		GA_RHI_TEMPLATE(Shader, const std::string& path, ShaderStage stage, std::vector<uint32_t>* compiledData);
	};

}