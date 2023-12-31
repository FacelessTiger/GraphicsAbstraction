#pragma once

#include <GraphicsAbstraction/Core/Core.h>
#include <GraphicsAbstraction/Renderer/Queue.h>

#include <memory>

namespace GraphicsAbstraction {

	class Window;
	class DeletionQueue;

	class GraphicsContext : public RefCounted
	{
	public:
		virtual ~GraphicsContext() = default;

		static void Init(uint32_t frameInFlightCount);

		inline static Ref<Queue> GetQueue(QueueType type) { return s_Instance->GetQueueImpl(type); };
		inline static void SetFrameInFlight(uint32_t fif) { s_Instance->SetFrameInFlightImpl(fif); };
	protected:
		virtual Ref<Queue> GetQueueImpl(QueueType type) = 0;
		virtual void SetFrameInFlightImpl(uint32_t fif) = 0;
	protected:
		static Ref<GraphicsContext> s_Instance;
	};
}