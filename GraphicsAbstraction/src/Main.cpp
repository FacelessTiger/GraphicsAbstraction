#include <GraphicsAbstraction/Core/Window.h>
#include <GraphicsAbstraction/Renderer/GraphicsContext.h>
#include <GraphicsAbstraction/Renderer/CommandPool.h>
#include <GraphicsAbstraction/Renderer/CommandBuffer.h>
#include <GraphicsAbstraction/Renderer/Renderpass.h>
#include <GraphicsAbstraction/Renderer/Fence.h>

int main()
{
	using namespace GraphicsAbstraction;

	std::shared_ptr<GraphicsContext> context = GraphicsContext::Create();
	std::shared_ptr<Window> window = Window::Create();

	std::shared_ptr<Swapchain> swapchain = Swapchain::Create(window, context);
	std::shared_ptr<CommandPool> commandPool = CommandPool::Create(context, QueueType::Graphics);
	std::shared_ptr<CommandBuffer> commandBuffer = commandPool->CreateCommandBuffer();

	Renderpass::Attachment colorAttachment;
	colorAttachment.LoadOperation = Renderpass::LoadOperation::Clear;
	colorAttachment.StoreOperation = Renderpass::StoreOperation::Store;
	colorAttachment.FinalImageLayout = Renderpass::ImageLayout::PresentSource;

	Renderpass::AttachmentReference colorAttachmentRef;
	colorAttachmentRef.AttachmentIndex = 0;
	colorAttachmentRef.ImageLayout = Renderpass::ImageLayout::ColorAttachmentOptimal;

	Renderpass::SubpassSpecification subpass;
	subpass.Bindpoint = Renderpass::PipelineBindpoint::Graphics;
	subpass.ColorAttachments = { colorAttachmentRef };

	Renderpass::Specification renderpassSpec;
	renderpassSpec.Attachments = { colorAttachment };
	renderpassSpec.Subpasses = { subpass };

	std::shared_ptr<Renderpass> renderpass = Renderpass::Create(renderpassSpec, context, swapchain);
	std::shared_ptr<Fence> fence = Fence::Create(context);

	uint32_t frameNumber = 0;
	while (!window->ShouldClose())
	{
		fence->Wait();

		uint32_t swapchainImageIndex = swapchain->AcquireNextImage();
		commandBuffer->Reset();
		commandBuffer->Begin();

		Vector4 clearColor(0.0f, 0.0f, (float)abs(sin(frameNumber / 120.0f)), 1.0f);
		renderpass->Begin(swapchain, commandBuffer, clearColor, swapchainImageIndex);

		renderpass->End(commandBuffer);
		commandBuffer->End();

		swapchain->SubmitCommandBuffer(commandBuffer, fence);
		swapchain->Present(swapchainImageIndex);

		window->OnUpdate();
		frameNumber++;
	}

	return 0;
}