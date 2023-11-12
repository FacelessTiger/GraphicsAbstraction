#pragma once

#include <GraphicsAbstraction/Renderer/Fence.h>

#include <vulkan/vulkan.h>

namespace GraphicsAbstraction {

	class VulkanContext;

	class VulkanFence : public Fence
	{
	public:
		VulkanFence(std::shared_ptr<GraphicsContext> context);
		virtual ~VulkanFence() = default;

		void Wait() const override;

		inline VkFence GetInternal() const { return m_Fence; }
	private:
		VkFence m_Fence;

		std::shared_ptr<VulkanContext> m_Context;
	};

}