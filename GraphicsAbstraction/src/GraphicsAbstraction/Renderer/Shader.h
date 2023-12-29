#pragma once

#include <memory>
#include <string>

#include <GraphicsAbstraction/Core/Log.h>
#include <GraphicsAbstraction/Core/Core.h>

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

	class Shader : public RefCounted
	{
	public:
		virtual ~Shader() = default;

		static Ref<Shader> Create(const std::string& path, ShaderStage stage);
	};

}