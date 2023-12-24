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

	class VulkanResourceHandle
	{
	public:
		VulkanResourceHandle(ResourceType type);
		~VulkanResourceHandle();

		uint32_t GetValue() const;
	private:
		ResourceType m_Type;
		mutable std::optional<uint32_t> m_ID;
	};

}