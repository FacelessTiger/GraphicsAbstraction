#include "QuadProcedure.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/packing.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>

namespace GraphicsAbstraction {

	void QuadProcedure::PreProcess(const RenderProcedurePrePayload& payload)
	{
		m_Vertex = Shader::Create("Assets/shaders/quadVertex.hlsl", ShaderStage::Vertex);
		m_Pixel = Shader::Create("Assets/shaders/quadPixel.hlsl", ShaderStage::Pixel);

		m_QuadBuffer = Buffer::Create(m_BufferSize, BufferUsage::StorageBuffer | BufferUsage::TransferDst, BufferFlags::DedicatedMemory);
		m_Vertex->WriteBuffer(m_QuadBuffer, 0);

		m_WhiteImage = Image::Create({ 1, 1 }, ImageFormat::R8G8B8A8_UNORM, ImageUsage::Sampled | ImageUsage::TransferDst);
		m_Sampler = Sampler::Create(Filter::Nearest, Filter::Nearest);

		uint32_t white = 0xFFFFFFFF;
		auto tempStaging = Buffer::Create(sizeof(uint32_t), BufferUsage::TransferSrc, BufferFlags::Mapped);
		tempStaging->SetData(&white, sizeof(uint32_t));

		payload.Pool->Reset();
		auto cmd = payload.Pool->Begin();
		cmd->CopyToImage(tempStaging, m_WhiteImage);
		payload.GraphicsQueue->Submit(cmd, nullptr, payload.Fence);
		payload.Fence->Wait();
	}

	void QuadProcedure::Process(const RenderProcedurePayload& payload)
	{
		auto cmd = payload.CommandBuffer;

		for (auto& change : m_QuadChanges)
			cmd->CopyToBuffer(change.stagingBuffer, m_QuadBuffer, change.stagingBuffer->GetSize(), 0, change.offset);
		m_QuadChanges.clear();

		cmd->BindShaders({ m_Vertex, m_Pixel });
		cmd->SetDepthTest(true, true, CompareOperation::LesserEqual);

		uint32_t handle = m_Sampler->GetHandle();
		cmd->PushConstant(&payload.ViewProjection, sizeof(glm::mat4), 16);
		cmd->PushConstant(&handle, sizeof(uint32_t), 80);
		cmd->BeginRendering(payload.Size, { payload.DrawImage }, payload.DepthImage);

		cmd->Draw(m_QuadCount * 6, 1, 0, 0);

		cmd->EndRendering();
	}

	uint32_t QuadProcedure::UploadQuad(const glm::vec3& position, const glm::vec2& scale, const glm::vec4& color, const std::shared_ptr<Texture>& texture)
	{
		uint32_t texureBinding = texture ? texture->GetImage()->GetHandle() : m_WhiteImage->GetHandle();
		uint32_t packedColor = glm::packUnorm4x8(color);
		uint32_t id = m_QuadCount++;

		QuadData data = {
			.scale = scale,
			.textureBinding = texureBinding,
			.color = packedColor,
			.position = position,
			.rotation = 0.0f,
		};

		// TODO: need to dynamically change m_Quadbuffer size
		/*uint32_t neededSize = ++m_QuadCount * sizeof(QuadData);
		if (neededSize >= m_BufferSize)
		{
			m_BufferSize = m_BufferSize + m_BufferSize / 2;

			auto oldBuffer = m_StagingBuffer;
			m_StagingBuffer = Buffer::Create(m_BufferSize, BufferUsage::TransferSrc, BufferFlags::Mapped);
			m_StagingBuffer->SetData(oldBuffer);

			auto stagingBuffer = Buffer::Create(m_BufferSize, BufferUsage::TransferSrc, BufferFlags::Mapped);
			stagingBuffer->SetData(m_QuadBuffer);
			m_QuadBuffer = Buffer::Create(m_BufferSize, BufferUsage::StorageBuffer | BufferUsage::TransferDst);
			m_Vertex->WriteBuffer(m_QuadBuffer, 0);
		}*/

		auto stagingBuffer = Buffer::Create(sizeof(QuadData), BufferUsage::TransferSrc, BufferFlags::Mapped);
		stagingBuffer->SetData(&data);

		QuadChange change = {
			.stagingBuffer = stagingBuffer,
			.offset = id * sizeof(QuadData)
		};
		m_QuadChanges.push_back(change);

		return id;
	}

	void QuadProcedure::UploadQuads(const std::vector<QuadUpload>& quads)
	{
		uint32_t firstID = m_QuadCount;
		m_QuadCount += (uint32_t)quads.size();

		std::vector<QuadData> data;
		data.reserve(quads.size());

		for (const auto& quad : quads)
		{
			data.push_back({
				.scale = quad.scale,
				.textureBinding = m_WhiteImage->GetHandle(),
				.color = glm::packUnorm4x8(quad.color),
				.position = quad.position,
				.rotation = 0.0f
			});
		}

		auto stagingBuffer = Buffer::Create((uint32_t)(sizeof(QuadData) * quads.size()), BufferUsage::TransferSrc, BufferFlags::Mapped);
		stagingBuffer->SetData(data.data());

		QuadChange change = {
			.stagingBuffer = stagingBuffer,
			.offset = firstID * sizeof(QuadData)
		};
		m_QuadChanges.push_back(change);
	}

	void QuadProcedure::UpdateQuadPosition(uint32_t id, const glm::vec3& position)
	{
		auto stagingBuffer = Buffer::Create(sizeof(glm::vec3), BufferUsage::TransferSrc, BufferFlags::Mapped);
		stagingBuffer->SetData(glm::value_ptr(position));

		QuadChange change = {
			.stagingBuffer = stagingBuffer,
			.offset = (id * sizeof(QuadData)) + offsetof(QuadData, position)
		};
		m_QuadChanges.push_back(change);
	}

	void QuadProcedure::UpdateQuadScale(uint32_t id, const glm::vec2& scale)
	{
		auto stagingBuffer = Buffer::Create(sizeof(glm::vec2), BufferUsage::TransferSrc, BufferFlags::Mapped);
		stagingBuffer->SetData(glm::value_ptr(scale));

		QuadChange change = {
			.stagingBuffer = stagingBuffer,
			.offset = (id * sizeof(QuadData)) + offsetof(QuadData, scale)
		};
		m_QuadChanges.push_back(change);
	}

	void QuadProcedure::UpdateQuadColor(uint32_t id, const glm::vec4& color)
	{
		uint32_t packedColor = glm::packUnorm4x8(color);

		auto stagingBuffer = Buffer::Create(sizeof(uint32_t), BufferUsage::TransferSrc, BufferFlags::Mapped);
		stagingBuffer->SetData(&packedColor);

		QuadChange change = {
			.stagingBuffer = stagingBuffer,
			.offset = (id * sizeof(QuadData)) + offsetof(QuadData, color)
		};
		m_QuadChanges.push_back(change);
	}

	void QuadProcedure::UpdateQuadRotation(uint32_t id, float rotation)
	{
		auto stagingBuffer = Buffer::Create(sizeof(float), BufferUsage::TransferSrc, BufferFlags::Mapped);
		stagingBuffer->SetData(&rotation);

		QuadChange change = {
			.stagingBuffer = stagingBuffer,
			.offset = (id * sizeof(QuadData)) + offsetof(QuadData, rotation)
		};
		m_QuadChanges.push_back(change);
	}

}