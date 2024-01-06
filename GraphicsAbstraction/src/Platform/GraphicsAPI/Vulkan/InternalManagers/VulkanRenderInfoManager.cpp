#include "VulkanRenderInfoManager.h"

#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanContext.h>
#include <GraphicsAbstraction/Debug/Instrumentor.h>

namespace GraphicsAbstraction {

	VulkanRenderInfoManager::VulkanRenderInfoManager(VulkanContext& context)
		: m_Context(context)
	{ }

	VulkanRenderInfoManager::~VulkanRenderInfoManager()
	{
		for (auto& [key, renderpass] : m_Renderpasses)
			vkDestroyRenderPass(m_Context.Device, renderpass, nullptr);
		for (auto& [key, framebuffer] : m_Framebuffers)
			vkDestroyFramebuffer(m_Context.Device, framebuffer, nullptr);
	}

	VulkanRenderInfo VulkanRenderInfoManager::GetRenderInfo(const VulkanRenderInfoKey& key)
	{
		GA_PROFILE_SCOPE();
		VkRenderPass renderpass;
		VkFramebuffer framebuffer;

		auto rIt = m_Renderpasses.find(key);
		if (rIt != m_Renderpasses.end()) renderpass = rIt->second;
		else
		{
			VkAttachmentReference depthReference;
			std::vector<VkAttachmentDescription> attachments;
			std::vector<VkAttachmentReference> colorReferences;
			attachments.reserve(key.ColorAttachments.size());
			colorReferences.reserve(key.ColorAttachments.size());

			bool depthExists = (key.DepthAttachment.Format != ImageFormat::Unknown);
			if (depthExists)
			{
				attachments.push_back({
					.format = Utils::GAImageFormatToVulkan(key.DepthAttachment.Format),
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				});

				depthReference = {
					.attachment = 0,
					.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
				};
			}

			for (uint32_t i = 0; i < key.ColorAttachments.size(); i++)
			{
				const auto& colorAttachment = key.ColorAttachments[i];
				if (colorAttachment.Format == ImageFormat::Unknown) break;

				VkAttachmentLoadOp loadOp = colorAttachment.InitialLayout ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachments.push_back({
					.format = Utils::GAImageFormatToVulkan(colorAttachment.Format),
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = loadOp,
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.initialLayout = colorAttachment.InitialLayout,
					.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
				});

				colorReferences.push_back({
					.attachment = i + depthExists,
					.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				});
			}

			VkSubpassDescription subpass = {
				.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount = (uint32_t)colorReferences.size(),
				.pColorAttachments = colorReferences.data()
			};
			if (depthExists) subpass.pDepthStencilAttachment = &depthReference;

			VkRenderPassCreateInfo renderpassCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
				.attachmentCount = (uint32_t)attachments.size(),
				.pAttachments = attachments.data(),
				.subpassCount = 1,
				.pSubpasses = &subpass
			};

			VK_CHECK(vkCreateRenderPass(m_Context.Device, &renderpassCreateInfo, nullptr, &renderpass));
			m_Renderpasses[key] = renderpass;
		}

		auto fIt = m_Framebuffers.find(key);
		if (fIt != m_Framebuffers.end()) framebuffer = fIt->second;
		else
		{
			std::vector<VkFramebufferAttachmentImageInfo> framebufferAttachments;
			std::vector<VkFormat> colorFormats;
			VkFormat depthFormat;

			framebufferAttachments.reserve(key.ColorAttachments.size());

			bool depthExists = (key.DepthAttachment.Format != ImageFormat::Unknown);
			if (depthExists)
			{
				depthFormat = Utils::GAImageFormatToVulkan(key.DepthAttachment.Format);

				framebufferAttachments.push_back(VkFramebufferAttachmentImageInfo{
					.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO,
					.usage = Utils::GAImageUsageToVulkan(key.DepthAttachment.Usage),
					.width = key.Width,
					.height = key.Height,
					.layerCount = 1,
					.viewFormatCount = 1,
					.pViewFormats = &depthFormat
				});
			}

			for (uint32_t i = 0; i < key.ColorAttachments.size(); i++)
			{
				const auto& colorAttachment = key.ColorAttachments[i];
				if (colorAttachment.Format == ImageFormat::Unknown) break;

				colorFormats.push_back(Utils::GAImageFormatToVulkan(colorAttachment.Format));
				framebufferAttachments.push_back(VkFramebufferAttachmentImageInfo{
					.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO,
					.usage = Utils::GAImageUsageToVulkan(colorAttachment.Usage),
					.width = key.Width,
					.height = key.Height,
					.layerCount = 1,
					.viewFormatCount = 1,
					.pViewFormats = &colorFormats.back()
				});
			}

			VkFramebufferAttachmentsCreateInfo attachmentInfo = {
				.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO,
				.attachmentImageInfoCount = (uint32_t)framebufferAttachments.size(),
				.pAttachmentImageInfos = framebufferAttachments.data()
			};

			VkFramebufferCreateInfo framebufferCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.pNext = &attachmentInfo,
				.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT,
				.renderPass = renderpass,
				.attachmentCount = (uint32_t)framebufferAttachments.size(),
				.width = key.Width,
				.height = key.Height,
				.layers = 1
			};

			vkCreateFramebuffer(m_Context.Device, &framebufferCreateInfo, nullptr, &framebuffer);
			m_Framebuffers[key] = framebuffer;
		}

		return { renderpass, framebuffer };
	}

	void VulkanRenderInfoManager::ClearFramebufferCache()
	{
		for (auto& [key, framebuffer] : m_Framebuffers)
			m_Context.GetFrameDeletionQueue().Push(framebuffer);
		m_Framebuffers.clear();
	}

}