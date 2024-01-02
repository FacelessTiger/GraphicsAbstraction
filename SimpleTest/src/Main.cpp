#include <GraphicsAbstraction/GraphicsAbstraction.h>

using namespace GraphicsAbstraction;
bool windowOpen = true;

void OnEvent(Event& e)
{
	if (e.GetEventType() == EventType::WindowClose) windowOpen = false;
}

int main()
{
	Ref<Window> window = Window::Create();
	window->SetEventCallback(std::move(OnEvent));

	GraphicsContext::Init(1);

	Ref<Swapchain> swapchain = Swapchain::Create(window, window->GetSize());
	Ref<Queue> queue = GraphicsContext::GetQueue(QueueType::Graphics);
	Ref<CommandPool> commandpool = CommandPool::Create(queue);
	Ref<Fence> fence = Fence::Create();

	while (windowOpen)
	{
		fence->Wait();
		GraphicsContext::SetFrameInFlight(0);
		queue->Acquire(swapchain, fence);

		auto cmd = commandpool->Reset()->Begin();
		cmd->Clear(swapchain->GetCurrent(), {0.0f, 0.0f, 1.0f, 1.0f});

		cmd->Present(swapchain);
		queue->Submit(cmd, fence, fence);
		queue->Present(swapchain, fence);

		window->OnUpdate();
	}
	
	GraphicsContext::Shutdown();
	return 0;
}