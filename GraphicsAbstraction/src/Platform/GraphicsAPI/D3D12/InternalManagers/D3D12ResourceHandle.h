#pragma once

#include <optional>

namespace GraphicsAbstraction {

	enum class ResourceType
	{
		Resource,
		Sampler
	};

	class D3D12ResourceHandle
	{
	public:
		D3D12ResourceHandle(ResourceType type = ResourceType::Resource);
		~D3D12ResourceHandle();

		uint32_t GetValue() const;
	private:
		ResourceType m_Type;
		mutable std::optional<uint32_t> m_ID;
	};

}