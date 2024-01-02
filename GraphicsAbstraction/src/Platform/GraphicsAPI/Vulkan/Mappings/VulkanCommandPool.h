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
		VulkanCommandPool(const Ref<Queue>& queue);
		virtual ~VulkanCommandPool();

		GraphicsAbstraction::CommandPool* Reset() override;
		Ref<CommandBuffer> Begin() const override;
	private:
		VulkanContextReference m_Context;
	};

}