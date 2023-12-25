#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <variant>
#include <unordered_map>
#include <vector>
#include <string>

namespace GraphicsAbstraction::Utils {

	struct AllocatedImage
	{
		VkImage Image;
		VmaAllocation Allocation;
	};

	struct AllocatedBuffer
	{
		VkBuffer Buffer;
		VmaAllocation Allocation;
		VmaAllocationInfo Info;
	};

	struct VulkanFeatureBuilder
	{
		std::unordered_map<VkStructureType, std::variant<VkPhysicalDeviceFeatures2, 
														VkPhysicalDeviceVulkan12Features,
														VkPhysicalDeviceShaderObjectFeaturesEXT,
														VkPhysicalDeviceExtendedDynamicStateFeaturesEXT,
														VkPhysicalDeviceExtendedDynamicState2FeaturesEXT,
														VkPhysicalDeviceExtendedDynamicState3FeaturesEXT,
														VkPhysicalDeviceDynamicRenderingFeatures>> deviceFeatures;
		std::vector<const char*> extensions;
		void* pNext = nullptr;

		VulkanFeatureBuilder()
		{
			auto& feature = deviceFeatures[VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2];
			auto& f = feature.emplace<VkPhysicalDeviceFeatures2>();
			f.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		}

		void AddExtension(const char* name)
		{
			extensions.push_back(name);
		}

		template<class T>
		void AddFeature(const T& feature)
		{
			auto& loc = deviceFeatures[feature.sType];

			auto& f = loc.emplace<T>(feature);
			f.pNext = pNext;
			pNext = &f;
		}

		const void* Build()
		{
			auto& f2 = std::get<VkPhysicalDeviceFeatures2>(deviceFeatures[VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2]);
			f2.pNext = pNext;

			return &f2;
		}
	};

	VkImageSubresourceRange ImageSubresourceRange(VkImageAspectFlags aspectMask);

}