#include <VulkanRHI.h>

namespace GraphicsAbstraction {

	Ref<Fence> Fence::Create()
	{
		auto fence = CreateRef<Fence>();
		fence->impl = new Impl<Fence>();
		return fence;
	}

	Fence::~Fence()
	{
		impl->Context->GetFrameDeletionQueue().Push(impl->TimelineSemaphore);
		delete impl;
	}

	void Fence::Wait() const
	{
		VkSemaphoreWaitInfo waitInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.semaphoreCount = 1,
			.pSemaphores = &impl->TimelineSemaphore,
			.pValues = &impl->Value
		};

		VK_CHECK(vkWaitSemaphores(impl->Context->Device, &waitInfo, UINT64_MAX));
	}

	Impl<Fence>::Impl()
		: Context(Impl<GraphicsContext>::Reference)
	{
		VkSemaphoreTypeCreateInfo timelineCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
			.pNext = nullptr,
			.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
			.initialValue = Value
		};

		VkSemaphoreCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = &timelineCreateInfo
		};

		VK_CHECK(vkCreateSemaphore(Context->Device, &createInfo, nullptr, &TimelineSemaphore));
	}

}