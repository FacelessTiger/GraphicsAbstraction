#pragma once

#include <GraphicsAbstraction/Core/Core.h>
#include <GraphicsAbstraction/Renderer/Renderpass.h>

#include <memory>

namespace GraphicsAbstraction {

	class GraphicsContext;
	class Shader;
	class CommandBuffer;

	class Pipeline
	{
	public:
		virtual ~Pipeline() = default;

		virtual void Bind(std::shared_ptr<CommandBuffer> cmd, Renderpass::PipelineBindpoint bindpoint) const = 0;

		static std::shared_ptr<Pipeline> Create(std::shared_ptr<GraphicsContext> context, std::shared_ptr<Shader> shader, std::shared_ptr<Renderpass> renderpass, uint32_t width, uint32_t height);
	};

}