#include "VulkanImage.h"

#include <GraphicsAbstraction/Core/Log.h>

#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanContext.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanCommandBuffer.h>
#include <GraphicsAbstraction/Debug/Instrumentor.h>

namespace GraphicsAbstraction {

	namespace Utils {

		VkFormat GAImageFormatToVulkan(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::R16G16B16A16_SFLOAT:	return VK_FORMAT_R16G16B16A16_SFLOAT;
				case ImageFormat::R8G8B8A8_UNORM:		return VK_FORMAT_R8G8B8A8_UNORM;
				case ImageFormat::D32_SFLOAT:			return VK_FORMAT_D32_SFLOAT;
			}

			GA_CORE_ASSERT(false, "Unknown image format!");
			return (VkFormat)0;
		}

		VkImageUsageFlags GAImageUsageToVulkan(ImageUsage usage)
		{
			VkImageUsageFlags ret = 0;

			if (usage & ImageUsage::ColorAttachment)		ret |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			if (usage & ImageUsage::DepthStencilAttachment)	ret |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			if (usage & ImageUsage::TransferSrc)			ret |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			if (usage & ImageUsage::TransferDst)			ret |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			if (usage & ImageUsage::Storage)				ret |= VK_IMAGE_USAGE_STORAGE_BIT;
			if (usage & ImageUsage::Sampled)				ret |= VK_IMAGE_USAGE_SAMPLED_BIT;

			return ret;
		}

	}

	VulkanImage::VulkanImage(const glm::vec2& size, ImageFormat format, ImageUsage usage)
		: m_Context(VulkanContext::GetReference()), Layout(VK_IMAGE_LAYOUT_UNDEFINED), Width((uint32_t)size.x), Height((uint32_t)size.y), Handle((usage& ImageUsage::Sampled) ? ResourceType::SampledImage : ResourceType::StorageImage)
	{
		Format = Utils::GAImageFormatToVulkan(format);
		Usage = Utils::GAImageUsageToVulkan(usage);

		Create();
	}

	VulkanImage::VulkanImage(VkImage image, VkImageView imageView, VkImageLayout imageLayout, VkFormat imageFormat, VkImageUsageFlags usage, uint32_t width, uint32_t height)
		: m_Context(VulkanContext::GetReference()), View(imageView), Layout(imageLayout), Format(imageFormat), Usage(usage), Width(width), Height(height), m_ExternalAllocation(true), Handle(ResourceType::StorageImage)
	{
		Image.Image = image;
	}

	VulkanImage::~VulkanImage()
	{
		Destroy();
	}

	void VulkanImage::CopyTo(const Ref<CommandBuffer>& cmd, const Ref<GraphicsAbstraction::Image>& other)
	{
		GA_PROFILE_SCOPE();

		auto& vulkanCommandBuffer = (VulkanCommandBuffer&)(*cmd);
		auto& vulkanImage = (VulkanImage&)(*other);

		TransitionLayout(vulkanCommandBuffer.CommandBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		vulkanImage.TransitionLayout(vulkanCommandBuffer.CommandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkImageBlit blitRegion{};
		blitRegion.srcOffsets[1].x = vulkanImage.Width;
		blitRegion.srcOffsets[1].y = vulkanImage.Height;
		blitRegion.srcOffsets[1].z = 1;

		blitRegion.dstOffsets[1].x = vulkanImage.Width;
		blitRegion.dstOffsets[1].y = vulkanImage.Height;
		blitRegion.dstOffsets[1].z = 1;

		blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRegion.srcSubresource.baseArrayLayer = 0;
		blitRegion.srcSubresource.layerCount = 1;
		blitRegion.srcSubresource.mipLevel = 0;

		blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRegion.dstSubresource.baseArrayLayer = 0;
		blitRegion.dstSubresource.layerCount = 1;
		blitRegion.dstSubresource.mipLevel = 0;

		vkCmdBlitImage(vulkanCommandBuffer.CommandBuffer, 
			Image.Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
			vulkanImage.Image.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			1, &blitRegion, VK_FILTER_LINEAR);
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
		VkImageCreateInfo imageInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType = VK_IMAGE_TYPE_2D, // TODO: specify somehow

			.format = Format,
			.extent = { Width, Height, 1 },

			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,

			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = Usage
		};

		VmaAllocationCreateInfo imgAllocInfo = {
			.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
		};
		vmaCreateImage(m_Context->Allocator, &imageInfo, &imgAllocInfo, &Image.Image, &Image.Allocation, nullptr);

		VkImageSubresourceRange range = {
			.aspectMask = (VkImageAspectFlags)((Usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT),
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		};
		VkImageViewCreateInfo imageViewInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = Image.Image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D, // TODO: specify, will be the same as image type above
			.format = Format,
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
		if (Usage & VK_IMAGE_USAGE_STORAGE_BIT)
		{
			VkDescriptorImageInfo imageInfo = {
				.imageView = View,
				.imageLayout = VK_IMAGE_LAYOUT_GENERAL
			};

			VkWriteDescriptorSet write = {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = m_Context->BindlessSet,
				.dstBinding = m_Context->STORAGE_IMAGE_BINDING,
				.dstArrayElement = Handle.GetValue(),
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				.pImageInfo = &imageInfo,
			};

			vkUpdateDescriptorSets(m_Context->Device, 1, &write, 0, nullptr);
		}
		else if (Usage & VK_IMAGE_USAGE_SAMPLED_BIT)
		{
			VkDescriptorImageInfo imageInfo = {
				.imageView = View,
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			};

			VkWriteDescriptorSet write = {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = m_Context->BindlessSet,
				.dstBinding = m_Context->SAMPLED_IMAGE_BINDING,
				.dstArrayElement = Handle.GetValue(),
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
			.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,

			.oldLayout = Layout,
			.newLayout = newLayout,

			.image = Image.Image,
			.subresourceRange = Utils::ImageSubresourceRange(aspectMask)
		};

		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
		Layout = newLayout;
	}

}