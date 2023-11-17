#pragma once

#include <GraphicsAbstraction/Renderer/CommandBuffer.h>

#include <vulkan/vulkan.h>
#include <memory>

namespace GraphicsAbstraction {

	class VulkanContext;

	class VulkanCommandBuffer : public CommandBuffer
	{
	public:
		VulkanCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue);
		virtual ~VulkanCommandBuffer() = default;

		void Reset() const override;
		void Submit(std::shared_ptr<Swapchain> swapchain, std::shared_ptr<Fence> fence) const override;
		void Draw(uint32_t vertexCount, uint32_t instanceCount) const override;
		void Present(std::shared_ptr<Swapchain> swapchain, uint32_t swapchainImageIndex) const override;

		void Begin() const override;
		void End() const override;

		inline VkCommandBuffer GetInternal() const { return m_CommandBuffer; }
	private:
		VkCommandBuffer m_CommandBuffer;
		VkQueue m_Queue;
	};

}