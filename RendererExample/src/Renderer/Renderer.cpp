#include "Renderer.h"

#include <Renderer/Procedures/RenderProcedure.h>
#include <glm/gtc/type_ptr.hpp>

namespace GraphicsAbstraction {

	struct ImageUpload
	{
		Ref<Buffer> src;
		Ref<Image> dst;
	};

	struct BufferUpload
	{
		Ref<Buffer> src, dst;
		uint32_t size, srcOffset, dstOffset;
	};

	struct DownsampleConstant
	{
		uint32_t srcImage;
		uint32_t dstImage;
		uint32_t sampler;
	};

	struct RendererData
	{
		static constexpr uint32_t FrameOverlap = 2;
		uint32_t FrameNumber = 0;
		glm::vec2 Size;
		bool SeperateDisplayImage;

		Ref<Queue> GraphicsQueue;
		Ref<Swapchain> Swapchain;
		Ref<Fence> Fence;
		Ref<Image> DrawImage, DepthImage, DisplayImage;
		Ref<CommandAllocator> CommandAllocators[FrameOverlap];

		Ref<Shader> ResizeShader;
		Ref<Sampler> ResizeSampler;

		std::vector<RenderProcedure*> RenderProcedures;
		std::function<void()> ImGuiCallback;

		std::vector<ImageUpload> ImageUploads;
		std::vector<BufferUpload> BufferUploads;
	};

	static RendererData* s_RendererData;

	void Renderer::Init(Ref<Window>& window, bool seperateDisplayImage)
	{
		s_RendererData = new RendererData();
		s_RendererData->Size = { 1920, 1080 };
		s_RendererData->SeperateDisplayImage = seperateDisplayImage;

		GraphicsContext::Init(RendererData::FrameOverlap);

		s_RendererData->GraphicsQueue = GraphicsContext::GetQueue(QueueType::Graphics);
		s_RendererData->Swapchain = Swapchain::Create(window, window->GetSize());
		s_RendererData->Fence = Fence::Create();
		s_RendererData->DrawImage = Image::Create({ 1920, 1080 }, ImageFormat::R16G16B16A16_SFLOAT, ImageUsage::Storage | ImageUsage::Sampled | ImageUsage::ColorAttachment | ImageUsage::TransferDst);
		s_RendererData->DepthImage = Image::Create({ 1920, 1080 }, ImageFormat::D32_SFLOAT, ImageUsage::DepthStencilAttachment);

		auto displayImageUsage = ImageUsage::Storage | ImageUsage::TransferSrc;
		if (seperateDisplayImage) displayImageUsage |= ImageUsage::Sampled;
		s_RendererData->DisplayImage = Image::Create(window->GetSize(), ImageFormat::R8G8B8A8_UNORM, displayImageUsage);

		s_RendererData->ResizeShader = Shader::Create("Assets/shaders/resizeImage.hlsl", ShaderStage::Compute, nullptr);
		s_RendererData->ResizeSampler = Sampler::Create(Filter::Nearest, Filter::Nearest);

		for (int i = 0; i < RendererData::FrameOverlap; i++)
			s_RendererData->CommandAllocators[i] = CommandAllocator::Create(s_RendererData->GraphicsQueue);

		ImGuiLayer::Init(s_RendererData->CommandAllocators[0], s_RendererData->Swapchain, window, s_RendererData->GraphicsQueue, s_RendererData->Fence);
	}

	void Renderer::Shutdown()
	{
		for (auto* procedure : s_RendererData->RenderProcedures)
			delete procedure;
		delete s_RendererData;

		ImGuiLayer::Shutdown();
	}

	void Renderer::SetImGuiCallback(std::function<void()> callback)
	{
		s_RendererData->ImGuiCallback = callback;
	}

	void Renderer::Resize(uint32_t width, uint32_t height)
	{
		s_RendererData->Swapchain->Resize(width, height);
		s_RendererData->DisplayImage->Resize({ width, height });
	}

	void Renderer::SetVsync(bool vsync)
	{
		s_RendererData->Swapchain->SetVsync(vsync);
	}

	void Renderer::CopyNextFrame(const Ref<Buffer>& srcBuffer, const Ref<Image>& dstImage)
	{
		s_RendererData->ImageUploads.push_back({ srcBuffer, dstImage });
	}

	void Renderer::CopyNextFrame(const Ref<Buffer>& srcBuffer, const Ref<Buffer>& dstBuffer, uint32_t size, uint32_t srcOffset, uint32_t dstOffset)
	{
		s_RendererData->BufferUploads.push_back({ srcBuffer, dstBuffer, size, srcOffset, dstOffset });
	}

	Ref<Image> Renderer::GetDrawImage()
	{
		return s_RendererData->DisplayImage;
	}

	void Renderer::AddProcedure(RenderProcedure* procedure)
	{
		s_RendererData->RenderProcedures.push_back(procedure);
	}

	void Renderer::PreProcess()
	{
		RenderProcedurePrePayload payload = {
			.GraphicsQueue = s_RendererData->GraphicsQueue,
			.DrawImage = s_RendererData->DrawImage,
			.Allocator = s_RendererData->CommandAllocators[0],
			.Fence = s_RendererData->Fence
		};

		for (auto* procedure : s_RendererData->RenderProcedures)
			procedure->PreProcess(payload);
	}

	void Renderer::Render(const EditorCamera& camera)
	{
		GA_PROFILE_SCOPE();

		auto& data = *s_RendererData;
		ImGuiLayer::BeginFrame();

		data.ImGuiCallback();

		uint32_t fif = data.FrameNumber++ % RendererData::FrameOverlap;
		data.Fence->Wait();
		GraphicsContext::SetFrameInFlight(fif);
		data.GraphicsQueue->Acquire(data.Swapchain, data.Fence);

		auto cmd = data.CommandAllocators[fif]->Reset()->Begin();
		for (auto& upload : data.ImageUploads)
			cmd->CopyToImage(upload.src, upload.dst);
		data.ImageUploads.clear();

		for (auto& upload : data.BufferUploads)
			cmd->CopyBufferRegion(upload.src, upload.dst, upload.size, upload.srcOffset, upload.dstOffset);
		data.BufferUploads.clear();

		cmd->SetViewport(data.Size);
		cmd->SetScissor(data.Size);
		cmd->Clear(data.DrawImage, { 0.0f, 0.0f, 0.0f, 1.0f });

		glm::mat4 viewProjection = camera.GetViewProjection();
		RenderProcedurePayload payload = {
			.DrawImage = data.DrawImage,
			.DepthImage = data.DepthImage,
			.CommandList = cmd,
			.Size = data.Size,
			.CameraPosition = camera.GetPosition(),
			.ViewProjection = viewProjection
		};

		for (auto* procedure : data.RenderProcedures)
			procedure->Process(payload);

		DownsampleConstant dpc = { data.DrawImage->GetSampledHandle(), data.DisplayImage->GetStorageHandle(), data.ResizeSampler->GetHandle() };
		cmd->PushConstant(dpc);
		cmd->BindShaders({ data.ResizeShader });
		cmd->Dispatch((uint32_t)std::ceil(data.DisplayImage->GetWidth() / 32.0f), (uint32_t)std::ceil(data.DisplayImage->GetHeight() / 32.0f), 1);
		cmd->RWResourceBarrier(data.DisplayImage);

		if (!data.SeperateDisplayImage) cmd->CopyToImage(data.DisplayImage, data.Swapchain->GetCurrent());
		ImGuiLayer::DrawFrame(cmd, data.Swapchain->GetCurrent());

		cmd->Present(data.Swapchain);

		data.GraphicsQueue->Submit(cmd, data.Fence, data.Fence);
		data.GraphicsQueue->Present(data.Swapchain, data.Fence);
	}

}