#pragma once

#include <GraphicsAbstraction/Renderer/Queue.h>

#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanContext.h>

namespace GraphicsAbstraction {

	class VulkanQueue : public Queue
	{
	public:
		VkQueue Queue;
		uint32_t QueueFamily;
	public:
		VulkanQueue(VulkanContext& context, VkQueue queue, uint32_t queueFamily);
		virtual ~VulkanQueue() = default;

		void Acquire(const std::shared_ptr<Swapchain>& swapchain, const std::shared_ptr<Fence>& fence) override;
		void Submit(const std::shared_ptr<CommandBuffer>& cmd, const std::shared_ptr<Fence>& wait, const std::shared_ptr<Fence>& signal) override;
		void Present(const std::shared_ptr<Swapchain>& swapchain, const std::shared_ptr<Fence>& wait) override;
	private:
		VulkanContext& m_Context;
	};

}