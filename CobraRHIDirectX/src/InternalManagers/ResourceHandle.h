#pragma once

#include <optional>

namespace GraphicsAbstraction {

	// Number represents stride
	enum class ResourceType
	{
		Sampler = 1,
		Resource = 2
	};

	class ResourceHandle
	{
	public:
		ResourceHandle(ResourceType type = ResourceType::Resource);
		~ResourceHandle();

		uint32_t GetValue() const;
	private:
		ResourceType m_Type;
		mutable std::optional<uint32_t> m_ID;
	};

}