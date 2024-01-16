#include "VulkanCommandAllocator.h"

#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanContext.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanQueue.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanImage.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanCommandList.h>

namespace GraphicsAbstraction {

	Ref<CommandAllocator> CommandAllocator::Create(const Ref<Queue>& queue)
	{
		return CreateRef<VulkanCommandAllocator>(queue);
	}

	VulkanCommandAllocator::VulkanCommandAllocator(const Ref<Queue>& queue)
		: m_Context(VulkanContext::GetReference())
	{
		auto& vulkanQueue = (VulkanQueue&)(*queue);

		VkCommandPoolCreateInfo commandPoolInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = nullptr,
			.queueFamilyIndex = vulkanQueue.QueueFamily
		};

		VK_CHECK(vkCreateCommandPool(m_Context->Device, &commandPoolInfo, nullptr, &CommandPool));

		VkCommandBufferAllocateInfo cmdAllocInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = CommandPool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1
		};

		VK_CHECK(vkAllocateCommandBuffers(m_Context->Device, &cmdAllocInfo, &MainCommandBuffer));
	}

	VulkanCommandAllocator::~VulkanCommandAllocator()
	{
		m_Context->GetFrameDeletionQueue().Push(CommandPool);
	}

	CommandAllocator* VulkanCommandAllocator::Reset()
	{
		vkResetCommandPool(m_Context->Device, CommandPool, 0);
		return this;
	}

	Ref<CommandList> VulkanCommandAllocator::Begin() const
	{
		VkCommandBufferBeginInfo info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
		};
		
		vkBeginCommandBuffer(MainCommandBuffer, &info);
		vkCmdBindDescriptorSets(MainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Context->BindlessPipelineLayout, 0, 1, &m_Context->BindlessSet, 0, nullptr);
		vkCmdBindDescriptorSets(MainCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_Context->BindlessPipelineLayout, 0, 1, &m_Context->BindlessSet, 0, nullptr);

		return CreateRef<VulkanCommandList>(MainCommandBuffer);
	}

}