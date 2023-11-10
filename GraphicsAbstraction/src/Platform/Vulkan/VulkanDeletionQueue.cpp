#include "VulkanDeletionQueue.h"

namespace GraphicsAbstraction {

	void VulkanDeletionQueue::PushFunction(std::function<void()>&& function)
	{
		m_Deletors.push_back(function);
	}

	void VulkanDeletionQueue::Flush()
	{
		for (auto it = m_Deletors.rbegin(); it != m_Deletors.rend(); it++)
			(*it)();

		m_Deletors.clear();
	}

}