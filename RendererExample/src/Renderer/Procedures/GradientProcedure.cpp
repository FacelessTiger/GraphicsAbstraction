#include "GradientProcedure.h"

#include <Renderer/Renderer.h>
#include <GraphicsAbstraction/Renderer/CommandList.h>

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace GraphicsAbstraction {

	struct CullMesh
	{
		glm::mat4 modelMatrix;
		DrawIndexedIndirectCommand command;
		Bounds bounds;
	};

	struct CullPushConstant
	{
		glm::mat4 viewProj;
		uint32_t inputBuffer;
		uint32_t outputBuffer;
		uint32_t countBuffer;
	};

	struct Object
	{
		glm::mat4 modelMatrix;
		uint32_t vertices;
	};

	struct PushConstant
	{
		glm::mat4 projection;
		glm::vec3 cameraPos;
		uint32_t vertices;
		uint32_t modelMatrices;
	};

	void GradientProcedure::PreProcess(RenderProcedurePrePayload& payload)
	{
		m_TriangleVertex = Shader::Create("Assets/shaders/TriangleVertex.hlsl", ShaderStage::Vertex, nullptr);
		m_TrianglePixel = Shader::Create("Assets/shaders/TrianglePixel.hlsl", ShaderStage::Pixel, nullptr);
		m_CullShader = Shader::Create("Assets/shaders/Cull.hlsl", ShaderStage::Compute, nullptr);
		m_Scene = ModelImporter::LoadModels("Assets/models/basicmesh.glb");

		m_ObjectBuffer = Buffer::Create((uint32_t)(sizeof(Object) * m_Scene.Meshes.size()), BufferUsage::StorageBuffer, BufferFlags::Mapped);
		m_CommandBuffer = Buffer::Create((uint32_t)(sizeof(DrawIndexedIndirectCommand) * m_Scene.Meshes.size()) + 4, BufferUsage::StorageBuffer | BufferUsage::IndirectBuffer | BufferUsage::TransferDst | BufferUsage::TransferSrc, BufferFlags::DeviceLocal);
		m_CullInputBuffer = Buffer::Create((uint32_t)(sizeof(CullMesh) * m_Scene.Meshes.size()), BufferUsage::StorageBuffer, BufferFlags::Mapped);

		uint32_t zero = 0;
		m_ZeroBuffer = Buffer::Create(sizeof(uint32_t), BufferUsage::StorageBuffer | BufferUsage::TransferSrc | BufferUsage::TransferDst, BufferFlags::Mapped);
		m_ZeroBuffer->SetData(&zero);

		for (uint32_t i = 0; i < m_Scene.Meshes.size(); i++)
		{
			Mesh& mesh = m_Scene.Meshes[i];

			m_Positions.push_back({0, 0, 0});
			glm::mat4 rotation = glm::toMat4(glm::quat({ 0.0f, 0.0f, 0.0f }));
			glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), { 0.0, 0.0f, 0.0f }) * rotation * glm::scale(glm::mat4(1.0f), { 1.0f, 1.0f, 1.0f });

			CullMesh cullMesh = {
				.modelMatrix = modelMatrix,
				.command = { mesh.Count, 1, mesh.StartIndex, 0, (uint32_t)i },
				.bounds = mesh.Bounds
			};
			m_CullInputBuffer->SetData(&cullMesh, sizeof(CullMesh), i * sizeof(CullMesh));

			Object object = {
				.modelMatrix = modelMatrix,
				.vertices = mesh.VertexBuffer->GetHandle()
			};
			m_ObjectBuffer->SetData(&object, sizeof(Object), sizeof(Object) * i);
		}
	}

	void GradientProcedure::Process(RenderProcedurePayload& payload)
	{
		auto cmd = payload.CommandList;

		if (m_CullDirty)
		{
			cmd->CopyBufferRegion(m_ZeroBuffer, m_CommandBuffer, sizeof(uint32_t));

			CullPushConstant cpc = { payload.ViewProjection, m_CullInputBuffer->GetHandle(), m_CommandBuffer->GetHandle() };
			cmd->PushConstant(cpc);
			cmd->BindShaders({ m_CullShader });
			cmd->Dispatch((uint32_t)std::ceil(m_Scene.Meshes.size() / 16.0f), 1, 1);
			cmd->RWResourceBarrier(m_CommandBuffer);

			//m_CullDirty = false;
		}

		cmd->BindShaders({ m_TriangleVertex, m_TrianglePixel });
		cmd->BeginRendering(payload.Size, { payload.DrawImage }, payload.DepthImage);

		cmd->EnableDepthTest(true, CompareOperation::GreaterEqual);
		cmd->SetFillMode(m_IsWireframe ? FillMode::Wireframe : FillMode::Solid);
		cmd->EnableColorBlend(Blend::SrcAlpha, Blend::OneMinusSrcAlpha, BlendOp::Add, Blend::One, Blend::Zero, BlendOp::Add);

		PushConstant pc = { payload.ViewProjection, payload.CameraPosition, m_ObjectBuffer->GetHandle() };
		cmd->PushConstant(pc);
		cmd->BindIndexBuffer(m_Scene.IndexBuffer);
		cmd->DrawIndexedIndirectCount(m_CommandBuffer, 4, m_CommandBuffer, 0, m_Scene.Meshes.size(), (uint32_t)sizeof(DrawIndexedIndirectCommand));

		cmd->EndRendering();

		OnImGui();
	}

	void GradientProcedure::OnImGui()
	{
		ImGui::Begin("Background");

		for (int i = 0; i < m_Positions.size(); i++)
		{
			ImGui::PushID(i);
			if (ImGui::DragFloat3("Position", glm::value_ptr(m_Positions[i])))
			{
				glm::mat4 rotation = glm::toMat4(glm::quat({ 0.0f, 0.0f, 0.0f }));
				glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), m_Positions[i]) * rotation * glm::scale(glm::mat4(1.0f), { 1.0f, 1.0f, 1.0f });
				m_ObjectBuffer->SetData(glm::value_ptr(modelMatrix), sizeof(glm::mat4), (sizeof(Object) * i) + offsetof(Object, modelMatrix));
			}
			ImGui::PopID();
		}

		ImGui::Checkbox("Wireframe", &m_IsWireframe);

		ImGui::End();
	}

}