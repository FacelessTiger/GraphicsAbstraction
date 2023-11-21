#pragma once

#include <GraphicsAbstraction/Renderer/PushConstant.h>

namespace GraphicsAbstraction {

	class VulkanPushConstant : public PushConstant
	{
	public:
		VulkanPushConstant(uint32_t offset, uint32_t size, ShaderStage stage)
			: m_Offset(offset), m_Size(size), m_Stage(stage)
		{ }
		virtual ~VulkanPushConstant() = default;

		void Push(std::shared_ptr<CommandBuffer> cmd, std::shared_ptr<Pipeline> pipeline, void* data) override;

		inline uint32_t GetOffset() const { return m_Offset; }
		inline uint32_t GetSize() const { return m_Size; }
		inline ShaderStage GetStage() const { return m_Stage; }
	private:
		uint32_t m_Offset, m_Size;
		ShaderStage m_Stage;
	};
}