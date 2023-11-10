#include <GraphicsAbstraction/Core/Window.h>
#include <GraphicsAbstraction/Renderer/GraphicsContext.h>

int main()
{
	using namespace GraphicsAbstraction;

	std::shared_ptr<GraphicsContext> context = GraphicsContext::Create();
	std::shared_ptr<Window> window = Window::Create();

	std::shared_ptr<Swapchain> swapchain = Swapchain::Create(window, context);

	while (!window->ShouldClose())
	{
		window->OnUpdate();
	}

	return 0;
}