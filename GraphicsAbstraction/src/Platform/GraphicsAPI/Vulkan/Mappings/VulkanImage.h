#pragma once

#include <GraphicsAbstraction/Renderer/Image.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanContext.h>
#include <Platform/GraphicsAPI/Vulkan/InternalManagers/VulkanResourceHandle.h>

#include <vulkan/vulkan.h>
#include <memory>

namespace GraphicsAbstraction {

	class VulkanContext;

	class VulkanImage : public Image
	{
	public:
		Utils::AllocatedImage Image;
		VkImageView View;
		VkImageLayout Layout;
		ImageFormat Format;
		ImageUsage Usage;

		uint32_t Width;
		uint32_t Height;

		VulkanResourceHandle SampledHandle = { ResourceType::SampledImage };
		VulkanResourceHandle StorageHandle = { ResourceType::StorageImage };
	public:
		VulkanImage(const glm::vec2& size, ImageFormat format, ImageUsage usage);
		VulkanImage(VkImage image, VkImageView imageView, VkImageLayout imageLayout, ImageFormat imageFormat, ImageUsage usage, uint32_t width, uint32_t height);
		virtual ~VulkanImage();

		void Resize(const glm::vec2& size) override;

		inline uint32_t GetSampledHandle() const override { return SampledHandle.GetValue(); }
		inline uint32_t GetStorageHandle() const override { return StorageHandle.GetValue(); }
		inline uint32_t GetWidth() const override { return Width; }
		inline uint32_t GetHeight() const override { return Height; }
		inline glm::vec2 GetSize() const { return { (float)Width, (float)Height }; };

		void TransitionLayout(VkCommandBuffer cmd, VkImageLayout newLayout);
	private:
		void Create();
		void Destroy();

		void UpdateDescriptor();
	private:
		Ref<VulkanContext> m_Context;
		bool m_ExternalAllocation = false;
	};

}