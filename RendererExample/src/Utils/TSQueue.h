#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

namespace GraphicsAbstraction {

	template <typename T>
	class TSQueue
	{
	public:
		void Push(T item)
		{
			std::unique_lock<std::mutex> lock(m_Mutex);

			m_Queue.push(item);
			m_Condition.notify_one();
		}

		T Pop()
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			m_Condition.wait(lock, [this]() { return !m_Queue.empty(); });

			T item = m_Queue.front();
			m_Queue.pop();
			return item;
		}

		bool Empty()
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			return m_Queue.empty();
		}
	private:
		std::queue<T> m_Queue;

		std::mutex m_Mutex;
		std::condition_variable m_Condition;
	};

}