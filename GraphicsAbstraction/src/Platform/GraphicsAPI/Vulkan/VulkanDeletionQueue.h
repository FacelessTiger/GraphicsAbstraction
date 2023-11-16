#pragma once

#include <functional>
#include <deque>

namespace GraphicsAbstraction {

	class VulkanContext;

	class VulkanDeletionQueue
	{
	public:
		void PushFunction(std::function<void(VulkanContext&)>&& function);
		void Flush(VulkanContext& context);
	private:
		std::deque<std::function<void(VulkanContext& context)>> m_Deletors;
	};

}