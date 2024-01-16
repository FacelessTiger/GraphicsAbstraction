#pragma once

#include <GraphicsAbstraction/Renderer/CommandAllocator.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanContext.h>

#include <vulkan/vulkan.h>

namespace GraphicsAbstraction {

	class VulkanCommandAllocator : public CommandAllocator
	{
	public:
		VkCommandPool CommandPool;
		VkCommandBuffer MainCommandBuffer;
	public:
		VulkanCommandAllocator(const Ref<Queue>& queue);
		virtual ~VulkanCommandAllocator();

		GraphicsAbstraction::CommandAllocator* Reset() override;
		Ref<CommandList> Begin() const override;
	private:
		Ref<VulkanContext> m_Context;
	};

}