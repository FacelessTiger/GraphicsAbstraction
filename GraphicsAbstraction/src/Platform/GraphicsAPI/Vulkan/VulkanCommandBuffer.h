#pragma once

#include <GraphicsAbstraction/Renderer/CommandBuffer.h>

#include <vulkan/vulkan.h>
#include <memory>

namespace GraphicsAbstraction {

	class VulkanContext;

	class VulkanCommandBuffer : public CommandBuffer
	{
	public:
		VulkanCommandBuffer(VkCommandBuffer commandBuffer);
		virtual ~VulkanCommandBuffer() = default;

		void Reset() const override;

		void Begin() const override;
		void End() const override;

		inline VkCommandBuffer GetInternal() const { return m_CommandBuffer; }
	private:
		VkCommandBuffer m_CommandBuffer;
	};

}