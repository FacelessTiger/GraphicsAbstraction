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

	void GradientProcedure::PreProcess(RenderProcedurePrePayload& payload)
	{
		m_Data.data1 = { 0.0f, 1.0f, 1.0f, 1.0f };
		m_Data.data2 = { 1.0f, 0.0f, 0.0f, 1.0f };

		m_Buffer = Buffer::Create(sizeof(Data), BufferUsage::StorageBuffer, BufferFlags::Mapped);
		m_Buffer->SetData(&m_Data);

		m_GradientShader = Shader::Create("Assets/shaders/gradient.hlsl", ShaderStage::Compute);
		m_ComputePC = { payload.DrawImage->GetHandle(), m_Buffer->GetHandle() };

		m_TriangleVertex = Shader::Create("Assets/shaders/triangleVertex.hlsl", ShaderStage::Vertex);
		m_TrianglePixel = Shader::Create("Assets/shaders/trianglePixel.hlsl", ShaderStage::Pixel);
		m_Meshes = ModelImporter::LoadModels("Assets/models/basicmesh.glb");
	}

	void GradientProcedure::Process(RenderProcedurePayload& payload)
	{
		auto cmd = payload.CommandBuffer;

		cmd->BindShaders({ m_GradientShader });
		cmd->PushConstant(m_ComputePC);
		cmd->Dispatch((uint32_t)std::ceil(payload.Size.x / 16.0f), (uint32_t)std::ceil(payload.Size.y / 16.0f), 1);

		cmd->BindShaders({ m_TriangleVertex, m_TrianglePixel });
		cmd->BeginRendering(payload.Size, { payload.DrawImage }, payload.DepthImage);

		PushConstant pc = { payload.ViewProjection, m_Meshes[2].VertexBuffer->GetHandle() };
		cmd->PushConstant(pc);
		cmd->SetDepthTest(true, true, CompareOperation::GreaterEqual);
		cmd->BindIndexBuffer(m_Meshes[2].IndexBuffer);
		cmd->EnableColorBlend(m_SrcBlend, m_DstBlend, BlendOp::Add, Blend::One, Blend::Zero, BlendOp::Add);
		cmd->DrawIndexed(m_Meshes[2].Surfaces[0].Count, 1, m_Meshes[2].Surfaces[0].StartIndex, 0, 0);
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

		ImGui::End();
	}

}