#pragma once

#include <memory>
#include <string>

#include <GraphicsAbstraction/Core/Log.h>

namespace GraphicsAbstraction {

	class GraphicsContext;
	class Buffer;
	class Image;

	enum class ShaderStage
	{
		Vertex,
		Pixel,
		Compute
	};

	class Shader
	{
	public:
		virtual ~Shader() = default;

		static std::shared_ptr<Shader> Create(const std::string& path, ShaderStage stage);
	};

}