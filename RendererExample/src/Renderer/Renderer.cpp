#include "Renderer.h"

#include <Assets/ShaderManager.h>
#include <Assets/AssetManager.h>
#include <Renderer/Texture.h>
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

	struct MaterialUpload
	{
		glm::vec3 albedoFactor;
		uint32_t albedoTexture;
		uint32_t metallicRoughnessTexture;
		float metallicFactor;
		float roughnessFactor;
		float ao;
	};

	struct Bounds
	{
		glm::vec3 origin;
		float sphereRadius;
		glm::vec3 extents;
		uint32_t transform;
	};

	struct CullMesh
	{
		Bounds bounds;
		DrawIndexedIndirectCommand command;
	};

	struct Object
	{
		uint32_t vertices;
		uint32_t material;
		uint32_t transform;
	};

	struct CullPushConstant
	{
		glm::mat4 viewProj;
		uint32_t inputBuffer;
		uint32_t inputCount;
		uint32_t outputBuffer;
		uint32_t models;
	};

	struct Light
	{
		glm::vec3 position;
		float padding;
		glm::vec3 color;
		float padding2;
	};

	struct PushConstant
	{
		glm::mat4 projection;
		glm::vec3 cameraPos;
		uint32_t objects;
		uint32_t materials;
		uint32_t lights;
		uint32_t lightCount;
		uint32_t models;
		uint32_t sampler;
	};

	struct RendererData
	{
		static constexpr uint32_t FrameOverlap = 2;
		uint32_t FrameNumber = 0, MaterialID = 0, TransformID = 0, PrimitiveID = 0, LightID = 0;
		glm::vec2 Size;
		bool SeperateDisplayImage;

		Ref<Queue> GraphicsQueue;
		Ref<Swapchain> Swapchain;
		Ref<Fence> Fence;
		Ref<Image> DrawImage, DepthImage, DisplayImage;
		Ref<CommandAllocator> CommandAllocators[FrameOverlap];

		Ref<Sampler> ResizeSampler;

		std::function<void()> ImGuiCallback;

		std::vector<ImageUpload> ImageUploads;
		std::vector<BufferUpload> BufferUploads;

		Ref<Buffer> MaterialBuffer, TransformBuffer, ObjectBuffer, CullInputBuffer, IndexBuffer, CommandBuffer, LightBuffer, ZeroBuffer;
		std::vector<Ref<Buffer>> VertexBuffers;
		uint32_t IndexOffset = 0;
	};

	static RendererData* s_RendererData;

	void Renderer::Init(Ref<Window>& window, bool seperateDisplayImage)
	{
		s_RendererData = new RendererData();
		s_RendererData->Size = { 1920, 1080 };
		s_RendererData->SeperateDisplayImage = seperateDisplayImage;

		GraphicsContext::Init(RendererData::FrameOverlap);
		ShaderManager::Init();

		s_RendererData->GraphicsQueue = GraphicsContext::GetQueue(QueueType::Graphics);
		s_RendererData->Swapchain = Swapchain::Create(window, window->GetSize());
		s_RendererData->Fence = Fence::Create();
		s_RendererData->DrawImage = Image::Create({ 1920, 1080 }, ImageFormat::R16G16B16A16_SFLOAT, ImageUsage::Storage | ImageUsage::Sampled | ImageUsage::ColorAttachment | ImageUsage::TransferDst);
		s_RendererData->DepthImage = Image::Create({ 1920, 1080 }, ImageFormat::D32_SFLOAT, ImageUsage::DepthStencilAttachment);

		auto displayImageUsage = ImageUsage::Storage | ImageUsage::TransferSrc;
		if (seperateDisplayImage) displayImageUsage |= ImageUsage::Sampled;
		s_RendererData->DisplayImage = Image::Create(window->GetSize(), ImageFormat::R8G8B8A8_UNORM, displayImageUsage);

		s_RendererData->ResizeSampler = Sampler::Create(Filter::Nearest, Filter::Nearest);

		s_RendererData->MaterialBuffer = Buffer::Create(1000 * sizeof(MaterialUpload), BufferUsage::StorageBuffer | BufferUsage::TransferDst, BufferFlags::DeviceLocal);
		s_RendererData->IndexBuffer = Buffer::Create(4'000'000 * sizeof(uint32_t), BufferUsage::IndexBuffer | BufferUsage::TransferDst, BufferFlags::DeviceLocal);
		s_RendererData->TransformBuffer = Buffer::Create(1000 * sizeof(glm::mat4), BufferUsage::StorageBuffer, BufferFlags::Mapped);
		s_RendererData->ObjectBuffer = Buffer::Create(10000 * sizeof(Object), BufferUsage::StorageBuffer, BufferFlags::Mapped);
		s_RendererData->CullInputBuffer = Buffer::Create(10000 * sizeof(CullMesh), BufferUsage::StorageBuffer, BufferFlags::Mapped);
		s_RendererData->CommandBuffer = Buffer::Create((10000 * sizeof(DrawIndexedIndirectCommand)) + 4, BufferUsage::StorageBuffer | BufferUsage::IndirectBuffer | BufferUsage::TransferDst, BufferFlags::DeviceLocal);
		s_RendererData->LightBuffer = Buffer::Create(10 * sizeof(Light), BufferUsage::StorageBuffer, BufferFlags::Mapped);

		uint32_t zero = 0;
		s_RendererData->ZeroBuffer = Buffer::Create(sizeof(uint32_t), BufferUsage::StorageBuffer | BufferUsage::TransferSrc | BufferUsage::TransferDst, BufferFlags::Mapped);
		s_RendererData->ZeroBuffer->SetData(&zero);

		for (int i = 0; i < RendererData::FrameOverlap; i++)
			s_RendererData->CommandAllocators[i] = CommandAllocator::Create(s_RendererData->GraphicsQueue);

		ImGuiLayer::Init(s_RendererData->CommandAllocators[0], s_RendererData->Swapchain, window, s_RendererData->GraphicsQueue, s_RendererData->Fence);
	}

	void Renderer::Shutdown()
	{
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

	uint32_t Renderer::UploadMaterial(const Ref<Material>& material)
	{
		Ref<Image> albedoTexture = ((Texture&)*AssetManager::GetAsset(material->AlbedoTexture)).GetImage();
		Ref<Image> metallicRoughnessTexture = ((Texture&)*AssetManager::GetAsset(material->MetallicRoughnessTexture)).GetImage();

		MaterialUpload materialUpload = {
			.albedoFactor = material->AlbedoFactor,
			.albedoTexture = albedoTexture->GetSampledHandle(),
			.metallicRoughnessTexture = metallicRoughnessTexture->GetSampledHandle(),
			.metallicFactor = material->MetallicFactor,
			.roughnessFactor = material->RoughnessFactor,
			.ao = material->AO
		};

		uint32_t handle = s_RendererData->MaterialID++;
		auto staging = Buffer::Create(sizeof(MaterialUpload), BufferUsage::TransferSrc, BufferFlags::Mapped);
		staging->SetData(&materialUpload);

		CopyNextFrame(staging, s_RendererData->MaterialBuffer, sizeof(MaterialUpload), 0, handle * sizeof(MaterialUpload));

		return handle;
	}

	uint32_t Renderer::UploadMesh(const std::vector<Submesh>& submeshes, const std::vector<uint32_t>& indices, const glm::mat4& initialModel)
	{
		uint32_t transformID = s_RendererData->TransformID++;
		s_RendererData->TransformBuffer->SetData(&initialModel, sizeof(glm::mat4), transformID * sizeof(glm::mat4));

		for (auto& submesh : submeshes)
		{
			uint32_t verticesSize = sizeof(Vertex) * submesh.Vertices.size();
			auto vertexBuffer = Buffer::Create(verticesSize, BufferUsage::StorageBuffer | BufferUsage::TransferDst, BufferFlags::DeviceLocal);
			auto staging = Buffer::Create(verticesSize, BufferUsage::TransferSrc, BufferFlags::Mapped);
			staging->SetData(submesh.Vertices.data());

			CopyNextFrame(staging, vertexBuffer, verticesSize);
			s_RendererData->VertexBuffers.push_back(vertexBuffer);

			uint32_t primitiveID = s_RendererData->PrimitiveID++;

			Object object = { vertexBuffer->GetHandle(), submesh.MaterialHandle, transformID };
			DrawIndexedIndirectCommand command(submesh.IndexCount, 1, s_RendererData->IndexOffset + submesh.IndexOffset, 0, primitiveID);

			glm::vec3 minPos = submesh.Vertices[0].position;
			glm::vec3 maxPos = submesh.Vertices[0].position;
			for (int i = 0; i < submesh.Vertices.size(); i++)
			{
				minPos = glm::min(minPos, submesh.Vertices[i].position);
				maxPos = glm::max(maxPos, submesh.Vertices[i].position);
			}

			CullMesh meshAsset = { .command = command };
			meshAsset.bounds.origin = (maxPos + minPos) / 2.0f;
			meshAsset.bounds.extents = (maxPos - minPos) / 2.0f;
			meshAsset.bounds.sphereRadius = glm::length(meshAsset.bounds.extents);
			meshAsset.bounds.transform = transformID;

			s_RendererData->ObjectBuffer->SetData(&object, sizeof(Object), primitiveID * sizeof(Object));
			s_RendererData->CullInputBuffer->SetData(&meshAsset, sizeof(CullMesh), primitiveID * sizeof(CullMesh));
		}

		auto indicesSize = indices.size() * sizeof(uint32_t);
		auto staging = Buffer::Create(indicesSize, BufferUsage::TransferSrc, BufferFlags::Mapped);
		staging->SetData(indices.data(), indicesSize);

		CopyNextFrame(staging, s_RendererData->IndexBuffer, indicesSize, 0, s_RendererData->IndexOffset * sizeof(uint32_t));
		s_RendererData->IndexOffset += indices.size();

		return transformID;
	}

	uint32_t Renderer::UploadLight(const glm::vec3& position, const glm::vec3& color)
	{
		uint32_t lightID = s_RendererData->LightID++;

		Light light = {
			.position = position,
			.color = color
		};
		s_RendererData->LightBuffer->SetData(&light, sizeof(Light), lightID * sizeof(Light));

		return lightID;
	}

	void GraphicsAbstraction::Renderer::UpdateLight(uint32_t lightHandle, const glm::vec3& position, const glm::vec3& color)
	{
		Light light = {
			.position = position,
			.color = color
		};
		s_RendererData->LightBuffer->SetData(&light, sizeof(Light), lightHandle * sizeof(Light));
	}

	void Renderer::UpdateTransform(uint32_t meshHandle, const glm::mat4& transform)
	{
		s_RendererData->TransformBuffer->SetData(&transform, sizeof(glm::mat4), meshHandle * sizeof(glm::mat4));
	}

	void Renderer::Render(const EditorCamera& camera)
	{
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

		Draw(cmd, camera);

		DownsampleConstant dpc = { data.DrawImage->GetSampledHandle(), data.DisplayImage->GetStorageHandle(), data.ResizeSampler->GetHandle() };
		cmd->PushConstant(dpc);
		cmd->BindShaders({ ShaderManager::Get("ResizeImage", ShaderStage::Compute) });
		cmd->Dispatch((uint32_t)std::ceil(data.DisplayImage->GetWidth() / 32.0f), (uint32_t)std::ceil(data.DisplayImage->GetHeight() / 32.0f), 1);
		cmd->RWResourceBarrier(data.DisplayImage);

		if (!data.SeperateDisplayImage) cmd->CopyToImage(data.DisplayImage, data.Swapchain->GetCurrent());
		ImGuiLayer::DrawFrame(cmd, data.Swapchain->GetCurrent());

		cmd->Present(data.Swapchain);

		data.GraphicsQueue->Submit(cmd, data.Fence, data.Fence);
		data.GraphicsQueue->Present(data.Swapchain, data.Fence);
	}

	void Renderer::Draw(Ref<CommandList>& cmd, const EditorCamera& camera)
	{
		static bool cullDirty = false;
		if (!cullDirty)
		{
			cmd->CopyBufferRegion(s_RendererData->ZeroBuffer, s_RendererData->CommandBuffer, sizeof(uint32_t));

			CullPushConstant cpc = { 
				camera.GetViewProjection(),
				s_RendererData->CullInputBuffer->GetHandle(),
				s_RendererData->PrimitiveID,
				s_RendererData->CommandBuffer->GetHandle(), 
				s_RendererData->TransformBuffer->GetHandle()
			};
			cmd->PushConstant(&cpc, 80, 0);
			cmd->BindShaders({ ShaderManager::Get("Cull", ShaderStage::Compute) });
			cmd->Dispatch((uint32_t)std::ceil((s_RendererData->PrimitiveID) / 16.0f), 1, 1);
			cmd->RWResourceBarrier(s_RendererData->CommandBuffer);

			//firstRun = true;
		}

		cmd->BindShaders({ ShaderManager::Get("TriangleVertex", ShaderStage::Vertex), ShaderManager::Get("TrianglePixel", ShaderStage::Pixel) });
		cmd->BeginRendering(s_RendererData->Size, { s_RendererData->DrawImage }, s_RendererData->DepthImage);

		cmd->EnableDepthTest(true, CompareOperation::GreaterEqual);
		cmd->SetFillMode(FillMode::Solid);
		cmd->EnableColorBlend(Blend::SrcAlpha, Blend::OneMinusSrcAlpha, BlendOp::Add, Blend::One, Blend::Zero, BlendOp::Add);

		PushConstant pc = { 
			camera.GetViewProjection(), 
			camera.GetPosition(),
			s_RendererData->ObjectBuffer->GetHandle(), 
			s_RendererData->MaterialBuffer->GetHandle(), 
			s_RendererData->LightBuffer->GetHandle(),
			s_RendererData->LightID,
			s_RendererData->TransformBuffer->GetHandle(),
			s_RendererData->ResizeSampler->GetHandle()
		};
		cmd->PushConstant(&pc, 100, 0);
		cmd->BindIndexBuffer(s_RendererData->IndexBuffer, IndexType::Uint32);
		cmd->DrawIndexedIndirectCount(s_RendererData->CommandBuffer, 4, s_RendererData->CommandBuffer, 0, s_RendererData->PrimitiveID, (uint32_t)sizeof(DrawIndexedIndirectCommand));

		cmd->EndRendering();
	}

}