#include <VulkanRHI.h>

namespace GraphicsAbstraction {

	Ref<CommandAllocator> CommandAllocator::Create(const Ref<Queue>& queue)
	{
		auto allocator = CreateRef<CommandAllocator>();
		allocator->impl = new Impl<CommandAllocator>(queue);
		return allocator;
	}

	CommandAllocator::~CommandAllocator()
	{
		impl->Context->GetFrameDeletionQueue().Push(impl->CommandPool);
		delete impl;
	}

	CommandAllocator* CommandAllocator::Reset()
	{
		vkResetCommandPool(impl->Context->Device, impl->CommandPool, 0);
		return this;
	}

	Ref<CommandList> CommandAllocator::Begin() const
	{
		VkCommandBufferBeginInfo info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
		};

		vkBeginCommandBuffer(impl->MainCommandBuffer, &info);
		vkCmdBindDescriptorSets(impl->MainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, impl->Context->BindlessPipelineLayout, 0, 1, &impl->Context->BindlessSet, 0, nullptr);
		vkCmdBindDescriptorSets(impl->MainCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, impl->Context->BindlessPipelineLayout, 0, 1, &impl->Context->BindlessSet, 0, nullptr);

		auto list = CreateRef<CommandList>();
		list->impl = new Impl<CommandList>(impl->MainCommandBuffer);
		return list;
	}

	Impl<CommandAllocator>::Impl(const Ref<Queue>& queue)
	{
		Context = Impl<GraphicsContext>::Reference;

		VkCommandPoolCreateInfo commandPoolInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = nullptr,
			.queueFamilyIndex = queue->impl->QueueFamily
		};

		VK_CHECK(vkCreateCommandPool(Context->Device, &commandPoolInfo, nullptr, &CommandPool));

		VkCommandBufferAllocateInfo cmdAllocInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = CommandPool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1
		};

		VK_CHECK(vkAllocateCommandBuffers(Context->Device, &cmdAllocInfo, &MainCommandBuffer));
	}

}