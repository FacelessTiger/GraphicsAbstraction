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

		virtual void WriteImage(const std::shared_ptr<Image> image, uint32_t index) = 0;
		virtual void WriteBuffer(const std::shared_ptr<Buffer> buffer, uint32_t index) = 0;

		static std::shared_ptr<Shader> Create(const std::string& path, ShaderStage stage);
	};

}