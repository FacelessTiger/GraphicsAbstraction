#include "GradientProcedure.h"

#include <Renderer/Renderer.h>
#include <GraphicsAbstraction/Renderer/CommandBuffer.h>

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace GraphicsAbstraction {

	struct PushConstant
	{
		glm::mat4 projection;
		uint32_t vertices;
	};

	void GradientProcedure::PreProcess(const RenderProcedurePrePayload& payload)
	{
		m_Data.data1 = { 0.0f, 1.0f, 1.0f, 1.0f };
		m_Data.data2 = { 1.0f, 0.0f, 0.0f, 1.0f };

		m_Buffer = Buffer::Create(sizeof(Data), BufferUsage::StorageBuffer, BufferFlags::Mapped);
		m_Buffer->SetData(&m_Data);

		m_GradientShader = Shader::Create("Assets/shaders/gradient.hlsl", ShaderStage::Compute);
		m_ComputePC = { payload.DrawImage->GetHandle(), m_Buffer->GetHandle() };

		m_TriangleVertex = Shader::Create("Assets/shaders/triangleVertex.hlsl", ShaderStage::Vertex);
		m_TrianglePixel = Shader::Create("Assets/shaders/trianglePixel.hlsl", ShaderStage::Pixel);

		std::vector<Vertex> vertices = {
			{ {  0.5f, -0.5f, 0.0f } },
			{ {  0.5f,  0.5f, 0.0f } },
			{ { -0.5f, -0.5f, 0.0f } },
			{ { -0.5f,  0.5f, 0.0f } }
		};
		uint32_t vertexBufferSize = (uint32_t)(vertices.size() * sizeof(Vertex));
		m_VertexBuffer = Buffer::Create(vertexBufferSize, BufferUsage::StorageBuffer | BufferUsage::TransferDst);

		std::vector<uint16_t> indices = { 0, 1, 2, 2, 1, 3 };
		uint32_t indexBufferSize = (uint16_t)(indices.size() * sizeof(uint16_t));
		m_IndexBuffer = Buffer::Create(indexBufferSize, BufferUsage::IndexBuffer | BufferUsage::TransferDst);

		auto staging = Buffer::Create(vertexBufferSize + indexBufferSize, BufferUsage::TransferSrc, BufferFlags::Mapped);
		staging->SetData(vertices.data(), vertexBufferSize);
		staging->SetData(indices.data(), indexBufferSize, vertexBufferSize);

		payload.Pool->Reset();
		auto cmd = payload.Pool->Begin();
		cmd->CopyToBuffer(staging, m_VertexBuffer, vertexBufferSize);
		cmd->CopyToBuffer(staging, m_IndexBuffer, indexBufferSize, vertexBufferSize);
		payload.GraphicsQueue->Submit(cmd, nullptr, payload.Fence);
		payload.Fence->Wait();
	}

	void GradientProcedure::Process(const RenderProcedurePayload& payload)
	{
		auto cmd = payload.CommandBuffer;

		cmd->BindShaders({ m_GradientShader });
		cmd->PushConstant(m_ComputePC);
		cmd->Dispatch((uint32_t)std::ceil(payload.Size.x / 16.0f), (uint32_t)std::ceil(payload.Size.y / 16.0f), 1);

		PushConstant pc = { payload.ViewProjection, m_VertexBuffer->GetHandle() };
		cmd->BindShaders({ m_TriangleVertex, m_TrianglePixel });
		cmd->PushConstant(pc);
		cmd->BeginRendering(payload.Size, { payload.DrawImage }, payload.DepthImage);

		cmd->SetDepthTest(true, true, CompareOperation::GreaterEqual);
		cmd->BindIndexBuffer(m_IndexBuffer);
		cmd->DrawIndexed(6, 1, 0, 0, 0);

		cmd->EndRendering();

		OnImGui();
	}

	void GradientProcedure::OnImGui()
	{
		ImGui::Begin("Background");
		bool dataChanged = false;

		if (ImGui::ColorEdit4("data1", glm::value_ptr(m_Data.data1))) dataChanged = true;
		if (ImGui::ColorEdit4("data2", glm::value_ptr(m_Data.data2))) dataChanged = true;

		if (dataChanged) m_Buffer->SetData(&m_Data);
		ImGui::End();
	}

}