#include "D3D12ResourceHandle.h"

#include <vector>
#include <unordered_map>

namespace GraphicsAbstraction {

	struct IDInfo
	{
		uint32_t IDCounter;
		std::vector<uint32_t> RecycledDescriptors;
	};

	static std::unordered_map<ResourceType, IDInfo> s_IDInfos = {
		{ ResourceType::Resource,	{ 0, std::vector<uint32_t>() }},
		{ ResourceType::Sampler,	{ 0, std::vector<uint32_t>() }},
	};

	D3D12ResourceHandle::D3D12ResourceHandle(ResourceType type)
		: m_Type(type)
	{ }

	D3D12ResourceHandle::~D3D12ResourceHandle()
	{
		if (m_ID.has_value()) s_IDInfos[m_Type].RecycledDescriptors.push_back(m_ID.value());
	}

	uint32_t D3D12ResourceHandle::GetValue() const
	{
		if (m_ID.has_value()) return m_ID.value();

		auto& recycledDescriptors = s_IDInfos[m_Type].RecycledDescriptors;
		if (!recycledDescriptors.empty())
		{
			m_ID = recycledDescriptors.back();
			recycledDescriptors.pop_back();

			return m_ID.value();
		}

		m_ID = s_IDInfos[m_Type].IDCounter++;
		return m_ID.value();
	}

}