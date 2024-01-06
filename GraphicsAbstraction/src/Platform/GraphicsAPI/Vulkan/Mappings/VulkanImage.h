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

		VulkanResourceHandle Handle;
	public:
		VulkanImage(const glm::vec2& size, ImageFormat format, ImageUsage usage);
		VulkanImage(VkImage image, VkImageView imageView, VkImageLayout imageLayout, ImageFormat imageFormat, ImageUsage usage, uint32_t width, uint32_t height);
		virtual ~VulkanImage();

		void CopyTo(const Ref<CommandBuffer>& cmd, const Ref<GraphicsAbstraction::Image>& other) override;
		void Resize(const glm::vec2& size) override;

		inline uint32_t GetHandle() const override { return Handle.GetValue(); }
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