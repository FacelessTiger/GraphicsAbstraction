#pragma once

#include <GraphicsAbstraction/Renderer/Image.h>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace GraphicsAbstraction {

	class VulkanContext;

	class VulkanImage : public Image
	{
	public:
		VulkanImage(std::shared_ptr<GraphicsContext> context, const Specification& spec);
		VulkanImage(VkImage image, VkImageView imageView, VkFormat format);
		virtual ~VulkanImage();

		inline VkImageView GetView() { return m_ImageView; }

		inline VkFormat GetFormat() const { return m_Format; }
		inline VkSampleCountFlagBits GetSamples() const { return m_Samples; }
	private:
		VkImageUsageFlags FormatToUsage(VkFormat format);
		VkSampleCountFlagBits SampleNumberToVulkan(uint32_t samples);
		VkImageAspectFlags UsageToAspect(VkImageUsageFlags usage);

		void CreateImage(VkImageUsageFlags usageFlags, VkExtent3D extent);
		void CreateImageView(VkImageAspectFlags aspect);
	private:
		std::shared_ptr<VulkanContext> m_Context;
		bool m_HandledExternally = false;

		VkImage m_Image;
		VkImageView m_ImageView;
		VmaAllocation m_Allocation;

		VkSampleCountFlagBits m_Samples;
		VkFormat m_Format;

		uint32_t m_Levels;
		uint32_t m_Layers;
	};

}