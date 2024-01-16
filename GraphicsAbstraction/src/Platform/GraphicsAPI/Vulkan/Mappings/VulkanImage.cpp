#include "VulkanImage.h"

#include <GraphicsAbstraction/Core/Log.h>

#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanContext.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanCommandList.h>
#include <GraphicsAbstraction/Debug/Instrumentor.h>

namespace GraphicsAbstraction {

	Ref<Image> Image::Create(const glm::vec2& size, ImageFormat format, ImageUsage usage)
	{
		return CreateRef<VulkanImage>(size, format, usage);
	}

	VulkanImage::VulkanImage(const glm::vec2& size, ImageFormat format, ImageUsage usage)
		: m_Context(VulkanContext::GetReference()), Layout(VK_IMAGE_LAYOUT_UNDEFINED), Format(format), Usage(usage), Width((uint32_t)size.x), Height((uint32_t)size.y)
	{
		Create();
	}

	VulkanImage::VulkanImage(VkImage image, VkImageView imageView, VkImageLayout imageLayout, ImageFormat imageFormat, ImageUsage usage, uint32_t width, uint32_t height)
		: m_Context(VulkanContext::GetReference()), View(imageView), Layout(imageLayout), Format(imageFormat), Usage(usage), Width(width), Height(height), m_ExternalAllocation(true)
	{
		Image.Image = image;
	}

	VulkanImage::~VulkanImage()
	{
		Destroy();
	}

	void VulkanImage::Resize(const glm::vec2& size)
	{
		Width = (uint32_t)size.x;
		Height = (uint32_t)size.y;
		Layout = VK_IMAGE_LAYOUT_UNDEFINED;

		Destroy();
		Create();
	}

	void VulkanImage::Create()
	{
		VkFormat vulkanFormat = Utils::GAImageFormatToVulkan(Format);

		VkImageCreateInfo imageInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType = VK_IMAGE_TYPE_2D, // TODO: specify somehow

			.format = vulkanFormat,
			.extent = { Width, Height, 1 },

			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,

			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = Utils::GAImageUsageToVulkan(Usage)
		};

		VmaAllocationCreateInfo imgAllocInfo = {
			.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
		};
		vmaCreateImage(m_Context->Allocator, &imageInfo, &imgAllocInfo, &Image.Image, &Image.Allocation, nullptr);

		VkImageSubresourceRange range = {
			.aspectMask = (VkImageAspectFlags)((Usage & ImageUsage::DepthStencilAttachment) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT),
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		};
		VkImageViewCreateInfo imageViewInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = Image.Image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D, // TODO: specify, will be the same as image type above
			.format = vulkanFormat,
			.subresourceRange = range
		};
		VK_CHECK(vkCreateImageView(m_Context->Device, &imageViewInfo, nullptr, &View));

		UpdateDescriptor();
	}

	void VulkanImage::Destroy()
	{
		m_Context->GetFrameDeletionQueue().Push(View);
		if (!m_ExternalAllocation)
			m_Context->GetFrameDeletionQueue().Push(Image);
	}

	void VulkanImage::UpdateDescriptor()
	{
		if (Usage & ImageUsage::Storage)
		{
			VkDescriptorImageInfo imageInfo = {
				.imageView = View,
				.imageLayout = VK_IMAGE_LAYOUT_GENERAL
			};

			VkWriteDescriptorSet write = {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = m_Context->BindlessSet,
				.dstBinding = m_Context->STORAGE_IMAGE_BINDING,
				.dstArrayElement = StorageHandle.GetValue(),
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				.pImageInfo = &imageInfo,
			};

			vkUpdateDescriptorSets(m_Context->Device, 1, &write, 0, nullptr);
		}

		if (Usage & ImageUsage::Sampled)
		{
			VkDescriptorImageInfo imageInfo = {
				.imageView = View,
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			};

			VkWriteDescriptorSet write = {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = m_Context->BindlessSet,
				.dstBinding = m_Context->SAMPLED_IMAGE_BINDING,
				.dstArrayElement = SampledHandle.GetValue(),
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
				.pImageInfo = &imageInfo,
			};

			vkUpdateDescriptorSets(m_Context->Device, 1, &write, 0, nullptr);
		}
	}

	void VulkanImage::TransitionLayout(VkCommandBuffer cmd, VkImageLayout newLayout)
	{
		if (Layout == newLayout) return;
		VkImageAspectFlags aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

		VkImageMemoryBarrier imageBarrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT,

			.oldLayout = Layout,
			.newLayout = newLayout,

			.image = Image.Image,
			.subresourceRange = Utils::ImageSubresourceRange(aspectMask)
		};

		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
		Layout = newLayout;
	}

}