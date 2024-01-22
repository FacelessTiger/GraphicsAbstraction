#pragma once

#include <optional>

namespace GraphicsAbstraction {

	enum class ResourceType
	{
		StorageBuffer,
		Sampler,
		StorageImage,
		SampledImage
	};

	class ResourceHandle
	{
	public:
		ResourceHandle(ResourceType type);
		~ResourceHandle();

		uint32_t GetValue() const;
	private:
		ResourceType m_Type;
		mutable std::optional<uint32_t> m_ID;
	};

}