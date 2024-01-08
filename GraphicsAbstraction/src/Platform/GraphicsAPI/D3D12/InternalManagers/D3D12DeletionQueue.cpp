#include "D3D12DeletionQueue.h"

namespace GraphicsAbstraction {

	D3D12DeletionQueue::D3D12DeletionQueue(D3D12Context& context)
		: m_Context(context)
	{ }

	D3D12DeletionQueue::~D3D12DeletionQueue()
	{

	}

	void D3D12DeletionQueue::Flush()
	{
		for (auto resource : m_Resources)
			resource.Reset();
		m_Resources.clear();

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
