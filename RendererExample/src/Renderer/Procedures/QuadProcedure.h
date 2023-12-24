#pragma once

#include <Renderer/Procedures/RenderProcedure.h>
#include <GraphicsAbstraction/Renderer/Buffer.h>
#include <GraphicsAbstraction/Renderer/Shader.h>
#include <GraphicsAbstraction/Renderer/Image.h>
#include <GraphicsAbstraction/Renderer/Sampler.h>
#include <Renderer/EditorCamera.h>
#include <Renderer/Texture.h>
#include <glm/glm.hpp>

namespace GraphicsAbstraction {

	struct QuadData
	{
		glm::vec2 scale;
		uint32_t textureBinding;
		uint32_t color;
		glm::vec3 position;
		float rotation;
	};

	struct QuadUpload
	{
		glm::vec2 scale;
		glm::vec3 position;
		glm::vec4 color;
	};

	struct QuadChange
	{
		std::shared_ptr<Buffer> stagingBuffer;
		uint32_t offset;
	};

	class QuadProcedure : public RenderProcedure
	{
	public:
		QuadProcedure() = default;
		virtual ~QuadProcedure() = default;

		void PreProcess(const RenderProcedurePrePayload& payload) override;
		void Process(const RenderProcedurePayload& payload) override;

		uint32_t UploadQuad(const glm::vec3& position, const glm::vec2& scale, const glm::vec4& color, const std::shared_ptr<Texture>& texture);
		void UploadQuads(const std::vector<QuadUpload>& quads);
		void UpdateQuadPosition(uint32_t id, const glm::vec3& position);
		void UpdateQuadScale(uint32_t id, const glm::vec2& scale);
		void UpdateQuadColor(uint32_t id, const glm::vec4& color);
		void UpdateQuadRotation(uint32_t id, float rotation);

		inline uint32_t GetQuadCount() const { return m_QuadCount; }
	private:
		std::shared_ptr<Shader> m_Vertex, m_Pixel;
		std::shared_ptr<Buffer> m_QuadBuffer;
		std::shared_ptr<Image> m_WhiteImage;
		std::shared_ptr<Sampler> m_Sampler;

		std::vector<QuadChange> m_QuadChanges;

		uint32_t m_QuadCount = 0;
		uint32_t m_BufferSize = 2'000'000 * sizeof(QuadData);
	};

}