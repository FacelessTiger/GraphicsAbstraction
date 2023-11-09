#pragma once

#include <vulkan/vulkan.h>

namespace VAP {

	class VulkanContext
	{
	public:
		VulkanContext();
	private:
		VkInstance m_Instance;
	};

}