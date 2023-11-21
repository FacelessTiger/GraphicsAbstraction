#pragma once

#include <GraphicsAbstraction/Core/Core.h>
#include <GraphicsAbstraction/Renderer/Renderpass.h>
#include <GraphicsAbstraction/Renderer/PushConstant.h>

#include <memory>

namespace GraphicsAbstraction {

	class GraphicsContext;
	class Shader;
	class CommandBuffer;
	class VertexBuffer;

	class Pipeline
	{
	public:
		struct Specification
		{
			std::vector<std::shared_ptr<Shader>> Shaders;
			std::vector<std::shared_ptr<VertexBuffer>> VertexBuffers;
			std::vector<std::shared_ptr<PushConstant>> PushConstants;

			std::shared_ptr<Renderpass> Renderpass;
			glm::vec2 Extent;
		};
	public:
		virtual ~Pipeline() = default;

		virtual void Bind(std::shared_ptr<CommandBuffer> cmd) const = 0;

		static std::shared_ptr<Pipeline> Create(std::shared_ptr<GraphicsContext> context, const Specification& spec);
	};

}