#pragma once

#include <Renderer/Procedures/RenderProcedure.h>
#include <GraphicsAbstraction/Renderer/Buffer.h>
#include <GraphicsAbstraction/Renderer/Shader.h>
#include <GraphicsAbstraction/Renderer/Image.h>
#include <GraphicsAbstraction/Renderer/Sampler.h>
#include <Renderer/EditorCamera.h>
#include <Renderer/Texture.h>
#include <glm/glm.hpp>

#include <queue>

namespace GraphicsAbstraction {

	struct QuadData
	{
		glm::vec2 scale;
		uint32_t textureBinding;
		uint32_t color;
		glm::vec3 position;
		float rotation;
	};

	struct QuadChange
	{
		uint32_t SrcOffset;
		uint32_t DstOffset;
		uint32_t Size;
		uint32_t BufferIndex;
	};

	struct RingBuffer
	{
		std::shared_ptr<Buffer> Buffer;
		uint32_t Offset = 0;
	};

	class QuadProcedure : public RenderProcedure
	{
	public:
		QuadProcedure() = default;
		virtual ~QuadProcedure() = default;

		void PreProcess(const RenderProcedurePrePayload& payload) override;
		void Process(const RenderProcedurePayload& payload) override;

		uint32_t UploadQuad(const glm::vec3& position, const glm::vec2& scale, const glm::vec4& color, const std::shared_ptr<Texture>& texture);
		void UpdateQuadPosition(uint32_t id, const glm::vec3& position);
		void UpdateQuadScale(uint32_t id, const glm::vec2& scale);
		void UpdateQuadColor(uint32_t id, const glm::vec4& color);
		void UpdateQuadRotation(uint32_t id, float rotation);

		inline uint32_t GetQuadCount() const { return m_QuadCount; }
	private:
		void Upload(const void* data, uint32_t id, uint32_t size, uint32_t offset = 0);
	private:
		std::shared_ptr<Shader> m_Vertex, m_Pixel;
		std::shared_ptr<Buffer> m_QuadBuffer;
		std::shared_ptr<Image> m_WhiteImage;
		std::shared_ptr<Sampler> m_Sampler;

		std::vector<RingBuffer> m_RingBuffers;
		std::queue<QuadChange> m_QuadChanges;

		uint32_t m_QuadCount = 0;
		uint32_t m_TotalUploadSize = 0;
		uint32_t m_BufferIndex = 0;

		uint32_t m_BufferSize = 2'000'000 * sizeof(QuadData);
		static const uint32_t m_RingBufferSize = 1'000'000 * sizeof(QuadData);
	};

}