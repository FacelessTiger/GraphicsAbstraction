#include "D3D12ResourceHandle.h"

#include <vector>
#include <unordered_map>

namespace GraphicsAbstraction {

	static std::vector<uint32_t> s_RecycledDescriptors;
	static uint32_t s_IDCounter = 0;

	D3D12ResourceHandle::~D3D12ResourceHandle()
	{
		if (m_ID.has_value()) s_RecycledDescriptors.push_back(m_ID.value());
	}

	uint32_t D3D12ResourceHandle::GetValue() const
	{
		if (m_ID.has_value()) return m_ID.value();
		if (!s_RecycledDescriptors.empty())
		{
			m_ID = s_RecycledDescriptors.back();
			s_RecycledDescriptors.pop_back();

			return m_ID.value();
		}

		m_ID = s_IDCounter++;
		return m_ID.value();
	}

}