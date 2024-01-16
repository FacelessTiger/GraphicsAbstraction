#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <variant>
#include <unordered_map>
#include <vector>
#include <string>

#include <GraphicsAbstraction/Renderer/Image.h>
#include <GraphicsAbstraction/Renderer/CommandList.h>

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
		std::unordered_map<VkStructureType, VkBaseInStructure*> deviceFeatures;
		std::vector<const char*> extensions;
		VkBaseInStructure* pNext = nullptr;

		~VulkanFeatureBuilder()
		{
			for (auto& [type, feature] : deviceFeatures)
				free(feature);
		}

		void AddExtension(const char* name)
		{
			extensions.push_back(name);
		}

		template<class T>
		void AddFeature(T&& feature)
		{
			auto& f = deviceFeatures[feature.sType];
			f = (VkBaseInStructure*)malloc(sizeof(T));
			*(T*)f = std::move(feature);
			f->pNext = pNext;

			pNext = f;
		}

		const void* GetChain() { return pNext; }
	};

	VkImageSubresourceRange ImageSubresourceRange(VkImageAspectFlags aspectMask);

	VkCompareOp GACompareOpToVulkan(CompareOperation op);
	VkBlendFactor GABlendToVulkan(Blend blend);
	VkBlendOp GABlendOpToVulkan(BlendOp blendOp);
	VkFormat GAImageFormatToVulkan(ImageFormat format);
	VkImageUsageFlags GAImageUsageToVulkan(ImageUsage usage);
	VkPolygonMode GAFillModeToVulkan(FillMode mode);

	ImageFormat VulkanImageFormatToGA(VkFormat format);
	ImageUsage VulkanImageUsageToGA(VkImageUsageFlags usage);

}