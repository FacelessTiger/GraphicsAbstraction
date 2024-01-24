#pragma once

#include <GraphicsAbstraction/Renderer/Shader.h>
#include <vector>

namespace GraphicsAbstraction {

	class DxcCompiler
	{
	public:
		static std::vector<uint32_t> CompileSource(const std::string& path, const std::string& source, ShaderStage stage, const std::vector<const wchar_t*>& additionalArguments, const std::vector<const wchar_t*>& defines);
	};

}