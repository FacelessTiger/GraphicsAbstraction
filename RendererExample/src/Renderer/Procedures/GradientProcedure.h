#pragma once

#include <Renderer/Procedures/RenderProcedure.h>
#include <GraphicsAbstraction/Renderer/Buffer.h>
#include <GraphicsAbstraction/Renderer/Shader.h>
#include <Assets/ModelImporter.h>
#include <glm/glm.hpp>

namespace GraphicsAbstraction {

	class GradientProcedure : public RenderProcedure
	{
	public:
		GradientProcedure() = default;
		virtual ~GradientProcedure() = default;

		void PreProcess(RenderProcedurePrePayload& payload) override;
		void Process(RenderProcedurePayload& payload) override;

		void OnImGui();
	private:
		bool m_IsWireframe = false;
		bool m_CullDirty = true;

		Ref<Buffer> m_CullInputBuffer, m_CommandBuffer, m_ZeroBuffer, m_ObjectBuffer, m_MaterialBuffer, m_LightBuffer;
		std::vector<glm::vec3> m_Positions, m_LightPositions, m_LightColors;
		Scene m_Scene;
	};

}