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

		void Acquire(const Ref<Swapchain>& swapchain, const Ref<Fence>& fence) override;
		void Submit(const Ref<CommandList>& cmd, const Ref<Fence>& wait, const Ref<Fence>& signal) override;
		void Present(const Ref<Swapchain>& swapchain, const Ref<Fence>& wait) override;
	private:
		VulkanContext& m_Context;
	};

}