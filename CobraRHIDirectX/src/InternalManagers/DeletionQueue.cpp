#include "DeletionQueue.h"

namespace GraphicsAbstraction {

	DeletionQueue::DeletionQueue(Impl<GraphicsContext>& context)
		: m_Context(context)
	{ }

	DeletionQueue::~DeletionQueue()
	{ }

	void DeletionQueue::Flush()
	{
		for (auto resource : m_AResources)
		{
			resource.Allocation.Reset();
			resource.Resource.Reset();
		}
		m_AResources.clear();

		for (auto allocator : m_Allocators)
			allocator.Reset();
		m_Allocators.clear();

		for (auto list : m_CommandLists)
			list.Reset();
		m_CommandLists.clear();

		for (auto fence : m_Fences)
			fence.Reset();
		m_Fences.clear();

		for (auto swapchains : m_Swapchains)
			swapchains.Reset();
		m_Swapchains.clear();
	}

}
