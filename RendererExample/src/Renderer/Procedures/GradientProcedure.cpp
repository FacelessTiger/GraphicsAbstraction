#include "GradientProcedure.h"

#include <Renderer/Renderer.h>
#include <GraphicsAbstraction/Renderer/CommandBuffer.h>

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace GraphicsAbstraction {

	struct PushConstant
	{
		glm::mat4 projection;
		uint32_t vertices;
		uint32_t modelMatrices;
	};

	void GradientProcedure::PreProcess(RenderProcedurePrePayload& payload)
	{
		m_Data.data1 = { 0.0f, 1.0f, 1.0f, 1.0f };
		m_Data.data2 = { 1.0f, 0.0f, 0.0f, 1.0f };

		m_Buffer = Buffer::Create(sizeof(Data), BufferUsage::StorageBuffer, BufferFlags::Mapped);
		m_Buffer->SetData(&m_Data);

		m_GradientShader = Shader::Create("Assets/shaders/gradient.hlsl", ShaderStage::Compute);
		m_ComputePC = { payload.DrawImage->GetStorageHandle(), m_Buffer->GetHandle() };

		m_TriangleVertex = Shader::Create("Assets/shaders/triangleVertex.hlsl", ShaderStage::Vertex);
		m_TrianglePixel = Shader::Create("Assets/shaders/trianglePixel.hlsl", ShaderStage::Pixel);
		m_Scene = ModelImporter::LoadModels("Assets/models/basicmesh.glb");

		m_ModelMatrixBuffer = Buffer::Create((uint32_t)(sizeof(glm::mat4) * m_Scene.Meshes.size()), BufferUsage::StorageBuffer, BufferFlags::Mapped);
		m_CommandBuffer = Buffer::Create((uint32_t)(sizeof(DrawIndexedIndirectCommand) * m_Scene.Meshes.size()), BufferUsage::StorageBuffer | BufferUsage::IndirectBuffer, BufferFlags::Mapped);

		for (uint32_t i = 0; i < m_Scene.Meshes.size(); i++)
		{
			Mesh& mesh = m_Scene.Meshes[i];

			DrawIndexedIndirectCommand command(mesh.Count, 1, mesh.StartIndex, 0, (uint32_t)i);
			m_CommandBuffer->SetData(&command, sizeof(DrawIndexedIndirectCommand), i * sizeof(DrawIndexedIndirectCommand));

			m_Positions.push_back({0, 0, 0});
			glm::mat4 rotation = glm::toMat4(glm::quat({ 0.0f, 0.0f, 0.0f }));
			glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), { 0.0, 0.0f, 0.0f }) * rotation * glm::scale(glm::mat4(1.0f), { 1.0f, 1.0f, 1.0f });
			m_ModelMatrixBuffer->SetData(glm::value_ptr(modelMatrix), sizeof(glm::mat4), sizeof(glm::mat4) * i);
		}
	}

	void GradientProcedure::Process(RenderProcedurePayload& payload)
	{
		auto cmd = payload.CommandBuffer;

		cmd->BindShaders({ m_GradientShader });
		cmd->PushConstant(m_ComputePC);
		cmd->Dispatch((uint32_t)std::ceil(payload.Size.x / 16.0f), (uint32_t)std::ceil(payload.Size.y / 16.0f), 1);

		cmd->BindShaders({ m_TriangleVertex, m_TrianglePixel });
		cmd->BeginRendering(payload.Size, { payload.DrawImage }, payload.DepthImage);

		cmd->EnableDepthTest(true, CompareOperation::GreaterEqual);
		cmd->EnableColorBlend(m_SrcBlend, m_DstBlend, BlendOp::Add, Blend::One, Blend::Zero, BlendOp::Add);

		PushConstant pc = { payload.ViewProjection, m_Scene.VertexBuffer->GetHandle(), m_ModelMatrixBuffer->GetHandle() };
		cmd->PushConstant(pc);
		cmd->BindIndexBuffer(m_Scene.IndexBuffer);
		cmd->DrawIndexedIndirect(m_CommandBuffer, 0, (uint32_t)m_Scene.Meshes.size(), (uint32_t)sizeof(DrawIndexedIndirectCommand));

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

		const char* items[] = { "Zero ", "One", "SrcAlpha", "DstAlpha", "OneMinusSrcAlpha" };
		static const char* srcItem = items[0];
		static const char* dstItem = items[0];

		ImGui::PushID("Src");
		if (ImGui::BeginCombo("Src Blending mode", srcItem))
		{
			for (int i = 0; i < IM_ARRAYSIZE(items); i++)
			{
				bool isSelected = (srcItem == items[i]);
				if (isSelected) ImGui::SetItemDefaultFocus();
				if (ImGui::Selectable(items[i], isSelected))
				{
					srcItem = items[i];
					m_SrcBlend = (Blend)i;
				}
			}

			ImGui::EndCombo();
		}
		ImGui::PopID();

		ImGui::PushID("Dst");
		if (ImGui::BeginCombo("Dst Blending mode", dstItem))
		{
			for (int i = 0; i < IM_ARRAYSIZE(items); i++)
			{
				bool isSelected = (dstItem == items[i]);
				if (isSelected) ImGui::SetItemDefaultFocus();
				if (ImGui::Selectable(items[i], isSelected))
				{
					dstItem = items[i];
					m_DstBlend = (Blend)i;
				}
			}

			ImGui::EndCombo();
		}
		ImGui::PopID();

		for (int i = 0; i < m_Positions.size(); i++)
		{
			ImGui::PushID(i);
			if (ImGui::DragFloat3("Position", glm::value_ptr(m_Positions[i])))
			{
				glm::mat4 rotation = glm::toMat4(glm::quat({ 0.0f, 0.0f, 0.0f }));
				glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), m_Positions[i]) * rotation * glm::scale(glm::mat4(1.0f), { 1.0f, 1.0f, 1.0f });
				m_ModelMatrixBuffer->SetData(glm::value_ptr(modelMatrix), sizeof(glm::mat4), sizeof(glm::mat4) * i);
			}
			ImGui::PopID();
		}

		ImGui::End();
	}

}