#include "VulkanDeletionQueue.h"

namespace GraphicsAbstraction {

	void VulkanDeletionQueue::PushFunction(std::function<void(VulkanContext&)>&& function)
	{
		m_Deletors.push_back(function);
	}

	void VulkanDeletionQueue::Flush(VulkanContext& context)
	{
		for (auto it = m_Deletors.rbegin(); it != m_Deletors.rend(); it++)
			(*it)(context);

		m_Deletors.clear();
	}

}