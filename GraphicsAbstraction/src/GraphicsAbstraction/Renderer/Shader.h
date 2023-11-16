#pragma once

#include <GraphicsAbstraction/Core/Core.h>

#include <memory>
#include <string>

namespace GraphicsAbstraction {

	class GraphicsContext;

	class Shader
	{
	public:
		virtual ~Shader() = default;

		static std::shared_ptr<Shader> Create(std::shared_ptr<GraphicsContext> context, const std::string& filepath);
	};

}