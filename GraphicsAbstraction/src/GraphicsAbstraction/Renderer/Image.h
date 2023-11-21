#pragma once

#include <glm/glm.hpp>
#include <memory>

namespace GraphicsAbstraction {

	class GraphicsContext;

	class Image
	{
	public:
		enum class Format
		{
			BGRA8SRGB,
			D32SFloat
		};

		struct Specification
		{
			Format Format = Format::BGRA8SRGB;
			glm::vec2 Size = { 0.0f, 0.0f };

			uint32_t Samples = 1;
			uint32_t Levels = 1;
			uint32_t Layers = 1;
		};
	public:
		virtual ~Image() = default;

		static std::shared_ptr<Image> Create(std::shared_ptr<GraphicsContext> context, const Specification& spec);
	};

}