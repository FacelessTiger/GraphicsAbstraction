#pragma once

#include <functional>
#include <deque>

namespace GraphicsAbstraction {

	class VulkanDeletionQueue
	{
	public:
		void PushFunction(std::function<void()>&& function);
		void Flush();
	private:
		std::deque<std::function<void()>> m_Deletors;
	};

}