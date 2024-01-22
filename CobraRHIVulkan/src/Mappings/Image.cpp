#include <VulkanRHI.h>

namespace GraphicsAbstraction {

	Ref<Image> Image::Create(const glm::vec2& size, ImageFormat format, ImageUsage usage)
	{
		auto image = CreateRef<Image>();
		image->impl = new Impl<Image>(size, format, usage);
		return image;
	}

	Image::~Image()
	{
		impl->Destroy();
		delete impl;
	}

	void Image::Resize(const glm::vec2& size)
	{
		impl->Width = (uint32_t)size.x;
		impl->Height = (uint32_t)size.y;
		impl->Layout = VK_IMAGE_LAYOUT_UNDEFINED;

		impl->Destroy();
		impl->ConstructImage();
	}

	uint32_t Image::GetSampledHandle() const { return impl->SampledHandle.GetValue(); }
	uint32_t Image::GetStorageHandle() const { return impl->StorageHandle.GetValue(); }
	uint32_t Image::GetWidth() const { return impl->Width; }
	uint32_t Image::GetHeight() const { return impl->Height; }
	glm::vec2 Image::GetSize() const { return { (float)impl->Width, (float)impl->Height }; }

	Impl<Image>::Impl(const glm::vec2& size, ImageFormat format, ImageUsage usage)
		: Context(Impl<GraphicsContext>::Reference), Layout(VK_IMAGE_LAYOUT_UNDEFINED), Format(format), Usage(usage), Width((uint32_t)size.x), Height((uint32_t)size.y)
	{
		ConstructImage();
	}

	Impl<Image>::Impl(VkImage image, VkImageView imageView, VkImageLayout imageLayout, ImageFormat imageFormat, ImageUsage usage, uint32_t width, uint32_t height)
		: Context(Impl<GraphicsContext>::Reference), View(imageView), Layout(imageLayout), Format(imageFormat), Usage(usage), Width((uint32_t)width), Height((uint32_t)height), ExternalAllocation(true)
	{
		Image.Image = image;
	}

	void Impl<Image>::TransitionLayout(VkCommandBuffer cmd, VkImageLayout newLayout)
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

	void Impl<Image>::ConstructImage()
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
		vmaCreateImage(Context->Allocator, &imageInfo, &imgAllocInfo, &Image.Image, &Image.Allocation, nullptr);

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
		VK_CHECK(vkCreateImageView(Context->Device, &imageViewInfo, nullptr, &View));

		UpdateDescriptor();
	}

	void Impl<Image>::Destroy()
	{
		Context->GetFrameDeletionQueue().Push(View);
		if (!ExternalAllocation)
			Context->GetFrameDeletionQueue().Push(Image);
	}

	void Impl<Image>::UpdateDescriptor()
	{
		if (Usage & ImageUsage::Storage)
		{
			VkDescriptorImageInfo imageInfo = {
				.imageView = View,
				.imageLayout = VK_IMAGE_LAYOUT_GENERAL
			};

			VkWriteDescriptorSet write = {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = Context->BindlessSet,
				.dstBinding = STORAGE_IMAGE_BINDING,
				.dstArrayElement = StorageHandle.GetValue(),
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				.pImageInfo = &imageInfo,
			};

			vkUpdateDescriptorSets(Context->Device, 1, &write, 0, nullptr);
		}

		if (Usage & ImageUsage::Sampled)
		{
			VkDescriptorImageInfo imageInfo = {
				.imageView = View,
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			};

			VkWriteDescriptorSet write = {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = Context->BindlessSet,
				.dstBinding = SAMPLED_IMAGE_BINDING,
				.dstArrayElement = SampledHandle.GetValue(),
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
				.pImageInfo = &imageInfo,
			};

			vkUpdateDescriptorSets(Context->Device, 1, &write, 0, nullptr);
		}
	}

}