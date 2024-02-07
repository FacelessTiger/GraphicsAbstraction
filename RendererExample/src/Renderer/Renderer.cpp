#include "Renderer.h"

#include <Core/Core.h>
#include <Assets/ShaderManager.h>
#include <Assets/AssetManager.h>
#include <Renderer/Texture.h>
#include <Renderer/Ringbuffer.h>
#include <glm/gtc/type_ptr.hpp>

namespace GraphicsAbstraction {

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

	struct Primitive
	{
		glm::vec3 origin;
		float sphereRadius;
		glm::vec3 extents;
		uint32_t indexCount;
		uint32_t indexOffset;
		uint32_t materialOffset; // TODO: I feel like this should be tied to the draw data instead
		uint32_t verticesOffset;
	};

	struct DrawData
	{
		uint32_t transformOffset;
		uint32_t primitiveOffset;
	};

	struct MeshContainer
	{
		struct PrimitiveContainer
		{
			VirtualAllocation Primitive;
			VirtualAllocation Vertices;
		};

		std::vector<PrimitiveContainer> Primitives;
	};

	struct ModelContainer
	{
		MeshContainer* Mesh;
		VirtualAllocation Transform;
	};

	struct CullPushConstant
	{
		glm::mat4 viewProj;
		uint32_t sceneBuffer;
		uint32_t inputCount;
		uint32_t inputOffset;
		uint32_t outputOffset;
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
		uint32_t scene;
		uint32_t drawsOffset;
		uint32_t lightOffset;
		uint32_t lightCount;
		uint32_t sampler;
	};

	struct CopyRegion
	{
		uint32_t SourceOffset;
		uint32_t DestOffset;
		uint32_t Size;
	};

	struct RendererData
	{
		static constexpr uint32_t FrameOverlap = 2;
		uint32_t FrameNumber = 0, DrawID = 0, LightID = 0;
		glm::vec2 Size;
		bool SeperateDisplayImage;

		Ref<Queue> GraphicsQueue;
		Ref<Swapchain> Swapchain;
		Ref<Fence> Fence;
		Ref<Image> DrawImage, DepthImage, DisplayImage;
		Ref<CommandAllocator> CommandAllocators[FrameOverlap];

		Ref<Sampler> ResizeSampler;
		std::function<void()> ImGuiCallback;

		std::vector<CopyRegion> CopyRegions;
		std::unordered_map<UUID, VirtualAllocation> Materials;
		std::unordered_map<UUID, ModelContainer> Models;
		std::unordered_map<UUID, MeshContainer> Meshes;
		std::unordered_map<UUID, uint32_t> Lights;
		Ref<VirtualAllocator> VirtualAllocator;
		VirtualAllocation CommandsAllocation;
		VirtualAllocation LightsAllocation;
		VirtualAllocation DrawInputAllocation;

		Ref<Buffer> SceneBuffer, IndexBuffer;
		Ringbuffer* Ringbuffer;
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

		s_RendererData->SceneBuffer = Buffer::Create(50'000'000, BufferUsage::StorageBuffer | BufferUsage::IndirectBuffer | BufferUsage::TransferDst | BufferUsage::TransferSrc, BufferFlags::DeviceLocal);
		s_RendererData->VirtualAllocator = VirtualAllocator::Create(50'000'000);
		s_RendererData->CommandsAllocation = s_RendererData->VirtualAllocator->Allocate(10000 * sizeof(DrawIndexedIndirectCommand));
		s_RendererData->DrawInputAllocation = s_RendererData->VirtualAllocator->Allocate(10000 * sizeof(DrawData));
		s_RendererData->LightsAllocation = s_RendererData->VirtualAllocator->Allocate(10 * sizeof(Light));

		s_RendererData->IndexBuffer = Buffer::Create(4'000'000 * sizeof(uint32_t), BufferUsage::IndexBuffer | BufferUsage::TransferDst, BufferFlags::DeviceLocal);
		s_RendererData->Ringbuffer = new Ringbuffer(512'000'000, Renderer::ImmediateSubmit);

		for (int i = 0; i < RendererData::FrameOverlap; i++)
			s_RendererData->CommandAllocators[i] = CommandAllocator::Create(s_RendererData->GraphicsQueue);

		ImGuiLayer::Init(s_RendererData->CommandAllocators[0], s_RendererData->Swapchain, window, s_RendererData->GraphicsQueue, s_RendererData->Fence);
	}

	void Renderer::Shutdown()
	{
		delete s_RendererData->Ringbuffer;
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

	void Renderer::CopyImage(void* src, Ref<Image> dstImage, uint32_t size)
	{
		s_RendererData->Ringbuffer->Write(src, dstImage, size);
	}

	Ref<Image> Renderer::GetDrawImage()
	{
		return s_RendererData->DisplayImage;
	}

	UUID Renderer::UploadMaterial(const Ref<Material>& material)
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

		VirtualAllocation allocation = s_RendererData->VirtualAllocator->Allocate(sizeof(MaterialUpload));
		s_RendererData->Ringbuffer->Write(&materialUpload, s_RendererData->SceneBuffer, sizeof(MaterialUpload), allocation.Offset);

		UUID uuid;
		s_RendererData->Materials[uuid] = std::move(allocation);
		return uuid;
	}

	UUID Renderer::UploadMesh(const std::vector<Submesh>& submeshes, const std::vector<uint32_t>& indices)
	{
		MeshContainer container;
		for (auto& submesh : submeshes)
		{
			uint32_t verticesSize = sizeof(Vertex) * submesh.Vertices.size();
			VirtualAllocation vertices = s_RendererData->VirtualAllocator->Allocate(verticesSize);
			s_RendererData->Ringbuffer->Write(submesh.Vertices.data(), s_RendererData->SceneBuffer, verticesSize, vertices.Offset);

			glm::vec3 minPos = submesh.Vertices[0].position;
			glm::vec3 maxPos = submesh.Vertices[0].position;
			for (int i = 0; i < submesh.Vertices.size(); i++)
			{
				minPos = glm::min(minPos, submesh.Vertices[i].position);
				maxPos = glm::max(maxPos, submesh.Vertices[i].position);
			}

			Primitive primitive = {
				.origin = (maxPos + minPos) / 2.0f,
				.extents = (maxPos - minPos) / 2.0f,
				.indexCount = submesh.IndexCount,
				.indexOffset = s_RendererData->IndexOffset + submesh.IndexOffset,
				.materialOffset = (uint32_t)s_RendererData->Materials[submesh.MaterialHandle].Offset,
				.verticesOffset = (uint32_t)vertices.Offset
			};
			primitive.sphereRadius = glm::length(primitive.extents);

			VirtualAllocation primitiveAllocation = s_RendererData->VirtualAllocator->Allocate(sizeof(Primitive));
			s_RendererData->Ringbuffer->Write(&primitive, s_RendererData->SceneBuffer, sizeof(Primitive), primitiveAllocation.Offset);
			container.Primitives.push_back({
				.Primitive = std::move(primitiveAllocation),
				.Vertices = std::move(vertices)
			});
		}

		auto indicesSize = indices.size() * sizeof(uint32_t);
		s_RendererData->Ringbuffer->Write(indices.data(), s_RendererData->IndexBuffer, indicesSize, s_RendererData->IndexOffset * sizeof(uint32_t));
		s_RendererData->IndexOffset += indices.size();

		UUID uuid;
		s_RendererData->Meshes[uuid] = std::move(container);
		return uuid;
	}

	UUID Renderer::UploadModel(UUID meshHandle, const glm::mat4& initialModel)
	{
		VirtualAllocation modelAllocation = s_RendererData->VirtualAllocator->Allocate(sizeof(glm::mat4));
		s_RendererData->Ringbuffer->Write(&initialModel, s_RendererData->SceneBuffer, sizeof(glm::mat4), modelAllocation.Offset);

		MeshContainer& mesh = s_RendererData->Meshes[meshHandle];
		for (auto& primitive : mesh.Primitives)
		{
			uint32_t primitiveID = s_RendererData->DrawID++;
			DrawData draw = {
				.transformOffset = (uint32_t)modelAllocation.Offset,
				.primitiveOffset = (uint32_t)primitive.Primitive.Offset
			};

			s_RendererData->Ringbuffer->Write(&draw, s_RendererData->SceneBuffer, sizeof(DrawData), (sizeof(DrawData) * primitiveID) + s_RendererData->DrawInputAllocation.Offset);
		}

		ModelContainer model = {
			.Mesh = &mesh,
			.Transform = std::move(modelAllocation)
		};

		UUID uuid;
		s_RendererData->Models.insert({ uuid, std::move(model) });
		return uuid;
	}

	UUID Renderer::UploadLight(const glm::vec3& position, const glm::vec3& color)
	{
		uint32_t lightID = s_RendererData->LightID++;

		Light light = {
			.position = position,
			.color = color
		};
		s_RendererData->Ringbuffer->Write(&light, s_RendererData->SceneBuffer, sizeof(Light), (lightID * sizeof(Light)) + s_RendererData->LightsAllocation.Offset);

		UUID uuid;
		s_RendererData->Lights[uuid] = lightID;
		return uuid;
	}

	void GraphicsAbstraction::Renderer::UpdateLight(UUID lightHandle, const glm::vec3& position, const glm::vec3& color)
	{
		uint32_t lightID = s_RendererData->Lights[lightHandle];

		Light light = {
			.position = position,
			.color = color
		};
		s_RendererData->Ringbuffer->Write(&light, s_RendererData->SceneBuffer, sizeof(Light), (lightID * sizeof(Light)) + s_RendererData->LightsAllocation.Offset);
	}

	void Renderer::UpdateTransform(UUID modelHandle, const glm::mat4& transform)
	{
		s_RendererData->Ringbuffer->Write(&transform, s_RendererData->SceneBuffer, sizeof(glm::mat4), s_RendererData->Models[modelHandle].Transform.Offset);
	}

	void Renderer::RemoveLight(UUID lightHandle)
	{
		uint32_t lightID = s_RendererData->Lights[lightHandle];
		uint32_t lastLightID = s_RendererData->LightID - 1;
		uint32_t lightsOffset = s_RendererData->LightsAllocation.Offset;

		s_RendererData->CopyRegions.push_back({
			.SourceOffset = (uint32_t)((sizeof(Light) * lastLightID) + lightsOffset),
			.DestOffset = (uint32_t)((sizeof(Light) * lightID) + lightsOffset),
			.Size = sizeof(Light)
		});

		// TODO: below is super inneficient, perhaps a bidirectional map?
		UUID replacementKey;
		for (auto& [key, value] : s_RendererData->Lights)
		{
			if (value == lastLightID)
			{
				replacementKey = key;
				break;
			}
		}
		//////////////////////////////////////////////////////////

		s_RendererData->Lights.erase(lightHandle);
		s_RendererData->Lights[replacementKey] = lightID;
		s_RendererData->LightID -= 1;
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
		data.Ringbuffer->Flush(cmd);

		for (auto& copyRegion : data.CopyRegions)
			cmd->CopyBufferRegion(data.SceneBuffer, data.SceneBuffer, copyRegion.Size, copyRegion.SourceOffset, copyRegion.DestOffset);
		data.CopyRegions.clear();

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

	void Renderer::ImmediateSubmit()
	{
		auto& data = *s_RendererData;

		uint32_t fif = data.FrameNumber % RendererData::FrameOverlap;
		auto cmd = data.CommandAllocators[fif]->Reset()->Begin();
		s_RendererData->Ringbuffer->Flush(cmd);

		data.GraphicsQueue->Submit(cmd, data.Fence, data.Fence);
		data.Fence->Wait();
	}

	void Renderer::Draw(Ref<CommandList>& cmd, const EditorCamera& camera)
	{
		uint64_t commandsOffset = s_RendererData->CommandsAllocation.Offset;
		uint64_t drawInputOffset = s_RendererData->DrawInputAllocation.Offset;

		static bool cullDirty = false;
		if (!cullDirty)
		{
			uint32_t zero = 0;
			s_RendererData->Ringbuffer->Write(&zero, s_RendererData->SceneBuffer, sizeof(uint32_t), commandsOffset);
			s_RendererData->Ringbuffer->Flush(cmd);

			CullPushConstant cpc = {
				camera.GetViewProjection(),
				s_RendererData->SceneBuffer->GetHandle(),
				s_RendererData->DrawID,
				drawInputOffset,
				commandsOffset
			};
			cmd->PushConstant(&cpc, 80, 0);
			cmd->BindShaders({ ShaderManager::Get("Cull", ShaderStage::Compute) });
			cmd->Dispatch((uint32_t)std::ceil((s_RendererData->DrawID) / 16.0f), 1, 1);
			cmd->RWResourceBarrier(s_RendererData->SceneBuffer);

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
			s_RendererData->SceneBuffer->GetHandle(), 
			drawInputOffset, 
			s_RendererData->LightsAllocation.Offset,
			s_RendererData->LightID,
			s_RendererData->ResizeSampler->GetHandle()
		};
		cmd->PushConstant(&pc, 96, 0);
		cmd->BindIndexBuffer(s_RendererData->IndexBuffer, IndexType::Uint32);
		cmd->DrawIndexedIndirectCount(s_RendererData->SceneBuffer, commandsOffset + 4, s_RendererData->SceneBuffer, commandsOffset, s_RendererData->DrawID, (uint32_t)sizeof(DrawIndexedIndirectCommand));

		cmd->EndRendering();
	}

}