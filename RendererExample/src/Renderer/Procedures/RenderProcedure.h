#pragma once

#include <GraphicsAbstraction/GraphicsAbstraction.h>

namespace GraphicsAbstraction {

	struct RenderProcedurePrePayload
	{
		Ref<Queue> GraphicsQueue;
		Ref<Image> DrawImage;
		Ref<CommandAllocator> Allocator;
		Ref<Fence> Fence;
	};

	struct RenderProcedurePayload
	{
		Ref<Image> DrawImage, DepthImage;
		Ref<CommandList> CommandList;
		const glm::vec2& Size;
		const glm::vec3& CameraPosition;
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