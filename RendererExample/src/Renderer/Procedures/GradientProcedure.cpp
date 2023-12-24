#include "GradientProcedure.h"

#include <Renderer/Renderer.h>
#include <GraphicsAbstraction/Renderer/CommandBuffer.h>

#include <imgui/imgui.h>

namespace GraphicsAbstraction {

	void GradientProcedure::PreProcess(const RenderProcedurePrePayload& payload)
	{
		m_Data.data1 = { 0.0f, 1.0f, 1.0f, 1.0f };
		m_Data.data2 = { 1.0f, 0.0f, 0.0f, 1.0f };

		m_Buffer = Buffer::Create(sizeof(Data), BufferUsage::StorageBuffer, BufferFlags::Mapped);
		m_Buffer->SetData(&m_Data);

		m_GradientShader = Shader::Create("Assets/shaders/gradient.hlsl", ShaderStage::Compute);
		m_GradientShader->WriteImage(payload.DrawImage, 0);
		m_GradientShader->WriteBuffer(m_Buffer, 1);

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

		std::vector<uint32_t> indices = { 0, 1, 2, 2, 1, 3 };
		uint32_t indexBufferSize = (uint32_t)(indices.size() * sizeof(uint32_t));
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

		m_TriangleVertex->WriteBuffer(m_VertexBuffer, 0);
	}

	void GradientProcedure::Process(const RenderProcedurePayload& payload)
	{
		auto cmd = payload.CommandBuffer;

		cmd->BindShaders({ m_GradientShader });
		cmd->Dispatch((uint32_t)std::ceil(1920.0f / 16.0f), (uint32_t)std::ceil(1080.0f / 16.0f), 1);

		cmd->BindShaders({ m_TriangleVertex, m_TrianglePixel });
		cmd->BeginRendering(payload.Size, { payload.DrawImage }, payload.DepthImage);

		cmd->SetDepthTest(true, true, CompareOperation::LesserEqual);
		cmd->BindIndexBuffer(m_IndexBuffer);
		cmd->DrawIndexed(6, 1, 0, 0, 0);

		cmd->EndRendering();

		OnImGui();
	}

	void GradientProcedure::OnImGui()
	{
		ImGui::Begin("Background");

		bool dataChanged = false;

		if (ImGui::ColorEdit4("data1", (float*)&m_Data.data1)) dataChanged = true;
		if (ImGui::ColorEdit4("data2", (float*)&m_Data.data2)) dataChanged = true;

		if (dataChanged) m_Buffer->SetData(&m_Data);

		//if (ImGui::Checkbox("Vsync", &m_Vsync))
		//	m_Swapchain->SetVsync(m_Vsync);
		//ImGui::Text("%fms %dPS", m_FrameTime, (int)(1.0 / m_FrameTime * 1000.0));

		ImGui::End();
	}

}