#include "Renderer.h"

#include <GraphicsAbstraction/Renderer/GraphicsContext.h>
#include <GraphicsAbstraction/ImGui/ImGuiLayer.h>
#include <GraphicsAbstraction/Debug/Instrumentor.h>
#include <GraphicsAbstraction/Renderer/Swapchain.h>
#include <GraphicsAbstraction/Renderer/CommandBuffer.h>
#include <GraphicsAbstraction/Renderer/Buffer.h>
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
		Ref<CommandPool> CommandPools[FrameOverlap];

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
		s_RendererData->DrawImage = Image::Create({ 1920, 1080 }, ImageFormat::R16G16B16A16_SFLOAT, ImageUsage::Storage | ImageUsage::ColorAttachment | ImageUsage::TransferSrc | ImageUsage::TransferDst);
		s_RendererData->DepthImage = Image::Create({ 1920, 1080 }, ImageFormat::D32_SFLOAT, ImageUsage::DepthStencilAttachment);
		if (seperateDisplayImage) s_RendererData->DisplayImage = Image::Create({ 1920, 1080 }, ImageFormat::R8G8B8A8_UNORM, ImageUsage::Sampled | ImageUsage::TransferDst);

		for (int i = 0; i < RendererData::FrameOverlap; i++)
			s_RendererData->CommandPools[i] = CommandPool::Create(s_RendererData->GraphicsQueue);

		ImGuiLayer::Init(s_RendererData->CommandPools[0], s_RendererData->Swapchain, window, s_RendererData->GraphicsQueue, s_RendererData->Fence);
	}

	void Renderer::Shutdown()
	{
		for (auto* procedure : s_RendererData->RenderProcedures)
			delete procedure;
		delete s_RendererData;

		ImGuiLayer::Shutdown();
		GraphicsContext::Shutdown();
	}

	void Renderer::SetImGuiCallback(std::function<void()> callback)
	{
		s_RendererData->ImGuiCallback = callback;
	}

	void Renderer::Resize(uint32_t width, uint32_t height)
	{
		s_RendererData->Swapchain->Resize(width, height);
		if (!s_RendererData->SeperateDisplayImage) s_RendererData->Size = { width, height };
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
			.Pool = s_RendererData->CommandPools[0],
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

		data.CommandPools[fif]->Reset();
		auto cmd = data.CommandPools[fif]->Begin();

		for (auto& upload : data.ImageUploads)
			cmd->CopyToImage(upload.src, upload.dst);
		data.ImageUploads.clear();

		for (auto& upload : data.BufferUploads)
			cmd->CopyToBuffer(upload.src, upload.dst, upload.size, upload.srcOffset, upload.dstOffset);
		data.BufferUploads.clear();

		cmd->SetViewport(data.Size);
		cmd->SetScissor(data.Size);
		cmd->Clear(data.DrawImage, { 0.0f, 0.0f, 0.0f, 1.0f });

		glm::mat4 viewProjection = camera.GetViewProjection();
		RenderProcedurePayload payload = {
			.DrawImage = data.DrawImage,
			.DepthImage = data.DepthImage,
			.CommandBuffer = cmd,
			.Size = data.Size,
			.ViewProjection = viewProjection
		};

		for (auto* procedure : data.RenderProcedures)
			procedure->Process(payload);

		auto copyDst = data.SeperateDisplayImage ? data.DisplayImage : data.Swapchain->GetCurrent();
		data.DrawImage->CopyTo(cmd, copyDst);
		ImGuiLayer::DrawFrame(cmd, data.Swapchain->GetCurrent());

		cmd->Present(data.Swapchain);

		data.GraphicsQueue->Submit(cmd, data.Fence, data.Fence);
		data.GraphicsQueue->Present(data.Swapchain, data.Fence);
	}

}