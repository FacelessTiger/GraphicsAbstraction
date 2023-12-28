#pragma once

#include <Renderer/Procedures/RenderProcedure.h>
#include <GraphicsAbstraction/Renderer/Buffer.h>
#include <GraphicsAbstraction/Renderer/Shader.h>
#include <glm/glm.hpp>

namespace GraphicsAbstraction {

	struct Data
	{
		glm::vec4 data1;
		glm::vec4 data2;
	};

	struct Vertex
	{
		glm::vec3 Position;
	};

	struct ComputePushConstant
	{
		uint32_t image;
		uint32_t buffer;
	};

	class GradientProcedure : public RenderProcedure
	{
	public:
		GradientProcedure() = default;
		virtual ~GradientProcedure() = default;

		void PreProcess(const RenderProcedurePrePayload& payload) override;
		void Process(const RenderProcedurePayload& payload) override;

		void OnImGui();
	private:
		Data m_Data;
		ComputePushConstant m_ComputePC;

		std::shared_ptr<Buffer> m_Buffer, m_VertexBuffer, m_IndexBuffer;
		std::shared_ptr<Shader> m_GradientShader, m_TriangleVertex, m_TrianglePixel;
	};

}