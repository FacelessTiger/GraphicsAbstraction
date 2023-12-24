#pragma once

#include <GraphicsAbstraction/Renderer/Fence.h>

#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanContext.h>

#include <vulkan/vulkan.h>

namespace GraphicsAbstraction {

	class VulkanFence : public Fence
	{
	public:
		VkSemaphore TimelineSemaphore;
		uint64_t Value = 0;
	public:
		VulkanFence();
		virtual ~VulkanFence();

		void Wait() override;
	private:
		VulkanContextReference m_Context;
	};

}