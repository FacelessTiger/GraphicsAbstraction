#pragma once

#include <cstdint>
#include <memory>

namespace GraphicsAbstraction {

	class CommandBuffer;
	class Pipeline;

	class PushConstant
	{
	public:
		enum class ShaderStage
		{
			Vertex
		};
	public:
		virtual ~PushConstant() = default;

		virtual void Push(std::shared_ptr<CommandBuffer> cmd, std::shared_ptr<Pipeline> pipeline, void* data) = 0;

		virtual inline uint32_t GetOffset() const = 0;
		virtual inline uint32_t GetSize() const = 0;
		virtual inline ShaderStage GetStage() const = 0;

		static std::shared_ptr<PushConstant> Create(uint32_t offset, uint32_t size, ShaderStage stage);
	};

}