#pragma once

#include <GraphicsAbstraction/Core/Core.h>
#include <GraphicsAbstraction/Renderer/Queue.h>

#include <memory>

namespace GraphicsAbstraction {

	class Window;
	class DeletionQueue;

	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;

		static void Init(uint32_t frameInFlightCount);
		inline static void Shutdown() { s_Instance->ShutdownImpl(); }

		inline static std::shared_ptr<Queue> GetQueue(QueueType type) { return s_Instance->GetQueueImpl(type); };
		inline static void SetFrameInFlight(uint32_t fif) { s_Instance->SetFrameInFlightImpl(fif); };
	protected:
		virtual void ShutdownImpl() = 0;

		virtual std::shared_ptr<Queue> GetQueueImpl(QueueType type) = 0;
		virtual void SetFrameInFlightImpl(uint32_t fif) = 0;
	private:
		static std::unique_ptr<GraphicsContext> s_Instance;
	};
}