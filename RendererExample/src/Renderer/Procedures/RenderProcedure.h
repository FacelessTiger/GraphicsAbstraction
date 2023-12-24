#pragma once

#include <GraphicsAbstraction/Renderer/Image.h>
#include <GraphicsAbstraction/Renderer/Queue.h>
#include <GraphicsAbstraction/Renderer/Fence.h>
#include <GraphicsAbstraction/Renderer/CommandPool.h>
#include <GraphicsAbstraction/Renderer/CommandBuffer.h>

namespace GraphicsAbstraction {

	struct RenderProcedurePrePayload
	{
		std::shared_ptr<Queue> GraphicsQueue;
		std::shared_ptr<Image> DrawImage;
		std::shared_ptr<CommandPool> Pool;
		std::shared_ptr<Fence> Fence;
	};

	struct RenderProcedurePayload
	{
		std::shared_ptr<Image> DrawImage, DepthImage;
		std::shared_ptr<CommandBuffer> CommandBuffer;
		const glm::vec2& Size;
		const glm::mat4& ViewProjection;
	};

	class RenderProcedure
	{
	public:
		virtual ~RenderProcedure() = default;

		virtual void PreProcess(const RenderProcedurePrePayload& payload) = 0;
		virtual void Process(const RenderProcedurePayload& payload) = 0;
	};

}