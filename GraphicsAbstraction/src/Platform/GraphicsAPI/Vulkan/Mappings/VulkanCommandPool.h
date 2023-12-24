#pragma once

#include <GraphicsAbstraction/Renderer/CommandPool.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanContext.h>

#include <vulkan/vulkan.h>

namespace GraphicsAbstraction {

	class VulkanCommandPool : public CommandPool
	{
	public:
		VkCommandPool CommandPool;
		VkCommandBuffer MainCommandBuffer;
	public:
		VulkanCommandPool(const std::shared_ptr<Queue>& queue);
		virtual ~VulkanCommandPool();

		void Reset() override;
		std::shared_ptr<CommandBuffer> Begin() override;
	private:
		VulkanContextReference m_Context;
	};

}