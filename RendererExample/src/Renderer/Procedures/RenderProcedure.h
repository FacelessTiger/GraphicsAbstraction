#pragma once

#include <GraphicsAbstraction/Renderer/Image.h>
#include <GraphicsAbstraction/Renderer/Queue.h>
#include <GraphicsAbstraction/Renderer/Fence.h>
#include <GraphicsAbstraction/Renderer/CommandPool.h>
#include <GraphicsAbstraction/Renderer/CommandBuffer.h>

namespace GraphicsAbstraction {

	struct RenderProcedurePrePayload
	{
		Ref<Queue> GraphicsQueue;
		Ref<Image> DrawImage;
		Ref<CommandPool> Pool;
		Ref<Fence> Fence;
	};

	struct RenderProcedurePayload
	{
		Ref<Image> DrawImage, DepthImage;
		Ref<CommandBuffer> CommandBuffer;
		const glm::vec2& Size;
		const glm::mat4& ViewProjection;
	};

	class RenderProcedure
	{
	public:
		virtual ~RenderProcedure() = default;

		virtual void PreProcess(RenderProcedurePrePayload& payload) = 0;
		virtual void Process(RenderProcedurePayload& payload) = 0;
	};

}