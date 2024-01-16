#include "QuadProcedure.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/packing.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>

namespace GraphicsAbstraction {

	struct PushConstant
	{
		glm::mat4 projection;
		uint32_t quadData;
		uint32_t sampler;
	};

	void QuadProcedure::PreProcess(RenderProcedurePrePayload& payload)
	{
		m_Vertex = Shader::Create("Assets/shaders/quadVertex.hlsl", ShaderStage::Vertex);
		m_Pixel = Shader::Create("Assets/shaders/quadPixel.hlsl", ShaderStage::Pixel);
		m_RingBuffers.reserve(10); // TODO: TEMP SOLUTION, BAD!!!!!!!

		m_QuadBuffer = Buffer::Create(m_BufferSize, BufferUsage::StorageBuffer | BufferUsage::TransferDst, BufferFlags::DeviceLocal);
		m_RingBuffers.push_back({ Buffer::Create(m_RingBufferSize, BufferUsage::TransferSrc, BufferFlags::Mapped) });

		m_WhiteImage = Image::Create({ 1, 1 }, ImageFormat::R8G8B8A8_UNORM, ImageUsage::Sampled | ImageUsage::TransferDst);
		m_Sampler = Sampler::Create(Filter::Nearest, Filter::Nearest);

		uint32_t white = 0xFFFFFFFF;
		auto tempStaging = Buffer::Create(sizeof(uint32_t), BufferUsage::TransferSrc, BufferFlags::Mapped);
		tempStaging->SetData(&white, sizeof(uint32_t));

		payload.Allocator->Reset();
		auto cmd = payload.Allocator->Begin();
		cmd->CopyToImage(tempStaging, m_WhiteImage);
		payload.GraphicsQueue->Submit(cmd, nullptr, payload.Fence);
		payload.Fence->Wait();
	}

	void QuadProcedure::Process(RenderProcedurePayload& payload)
	{
		auto cmd = payload.CommandList;

		while (!m_QuadChanges.Empty())
		{
			auto change = m_QuadChanges.Pop();
			cmd->CopyBufferRegion(m_RingBuffers[change.BufferIndex].Buffer, m_QuadBuffer, change.Size, change.SrcOffset, change.DstOffset);
		}
		m_TotalUploadSize = 0;
		m_BufferIndex = 0;

		cmd->BindShaders({ m_Vertex, m_Pixel });
		cmd->EnableDepthTest(true, CompareOperation::GreaterEqual);

		PushConstant pc = { payload.ViewProjection, m_QuadBuffer->GetHandle(), m_Sampler->GetHandle() };
		cmd->PushConstant(pc);
		cmd->BeginRendering(payload.Size, { payload.DrawImage }, payload.DepthImage);

		cmd->Draw(m_QuadCount * 6, 1, 0, 0);

		cmd->EndRendering();
	}

	uint32_t QuadProcedure::UploadQuad(const glm::vec3& position, const glm::vec2& scale, const glm::vec4& color, const std::shared_ptr<Texture>& texture)
	{
		uint32_t texureBinding = texture ? texture->GetImage()->GetSampledHandle() : m_WhiteImage->GetSampledHandle();
		uint32_t packedColor = glm::packUnorm4x8(color);
		uint32_t id = m_QuadCount++;

		QuadData data = {
			.scale = scale,
			.textureBinding = texureBinding,
			.color = packedColor,
			.position = position,
			.rotation = 0.0f
		};

		// TODO: need to dynamically change m_Quadbuffer size
		Upload(&data, id, sizeof(QuadData));
		return id;
	}

	void QuadProcedure::UpdateQuadPosition(uint32_t id, const glm::vec3& position)
	{
		Upload(glm::value_ptr(position), id, sizeof(glm::vec3), offsetof(QuadData, position));
	}

	void QuadProcedure::UpdateQuadScale(uint32_t id, const glm::vec2& scale)
	{
		Upload(glm::value_ptr(scale), id, sizeof(glm::vec2), offsetof(QuadData, scale));
	}

	void QuadProcedure::UpdateQuadColor(uint32_t id, const glm::vec4& color)
	{
		uint32_t packedColor = glm::packUnorm4x8(color);
		Upload(&packedColor, id, sizeof(uint32_t), offsetof(QuadData, color));
	}

	void QuadProcedure::UpdateQuadRotation(uint32_t id, float rotation)
	{
		Upload(&rotation, id, sizeof(float), offsetof(QuadData, rotation));
	}

	void QuadProcedure::Upload(const void* data, uint32_t id, uint32_t size, uint32_t offset)
	{
		m_TotalUploadSize += size;
		if (m_TotalUploadSize >= m_RingBufferSize)
		{
			m_BufferIndex++;
			m_TotalUploadSize = 0;

			if (m_BufferIndex > (m_RingBuffers.size() - 1)) m_RingBuffers.push_back({ Buffer::Create(m_RingBufferSize, BufferUsage::TransferSrc, BufferFlags::Mapped) });
		}

		uint32_t srcOffset = m_RingBuffers[m_BufferIndex].Offset;
		m_RingBuffers[m_BufferIndex].Offset = (m_RingBuffers[m_BufferIndex].Offset + size) % m_RingBufferSize;

		QuadChange change = {
			.SrcOffset = srcOffset,
			.DstOffset = (id * sizeof(QuadData)) + offset,
			.Size = size,
			.BufferIndex = m_BufferIndex
		};

		m_RingBuffers[m_BufferIndex].Buffer->SetData(data, change.Size, change.SrcOffset);
		m_QuadChanges.Push(change);
	}

}