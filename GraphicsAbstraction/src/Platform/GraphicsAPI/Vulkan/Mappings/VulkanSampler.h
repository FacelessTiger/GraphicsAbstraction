#pragma once

#include <GraphicsAbstraction/Renderer/Sampler.h>
#include <vulkan/vulkan.h>

#include <Platform/GraphicsAPI/Vulkan/InternalManagers/VulkanResourceHandle.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanContext.h>

namespace GraphicsAbstraction {

	class VulkanSampler : public Sampler
	{
	public:
		VkSampler Sampler;
		VulkanResourceHandle Handle = { ResourceType::Sampler };
	public:
		VulkanSampler(Filter min, Filter mag);
		virtual ~VulkanSampler();

		inline uint32_t GetHandle() const override { return Handle.GetValue(); }
	private:
		VulkanContextReference m_Context;
	};

}