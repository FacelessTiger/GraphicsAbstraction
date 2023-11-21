#include "VulkanRenderpass.h"

#include <Platform/GraphicsAPI/Vulkan/VulkanContext.h>
#include <Platform/GraphicsAPI/Vulkan/VulkanSwapchain.h>
#include <Platform/GraphicsAPI/Vulkan/VulkanCommandBuffer.h>

#include <GraphicsAbstraction/Core/Core.h>
#include <GraphicsAbstraction/Debug/Instrumentor.h>

namespace GraphicsAbstraction {

	VulkanRenderpass::VulkanRenderpass(std::shared_ptr<GraphicsContext> context, const Specification& spec)
	{
		GA_PROFILE_SCOPE();

		m_Context = std::dynamic_pointer_cast<VulkanContext>(context);

		InitRenderpass(spec);
		CreateFramebuffers(spec);
		//Recreate(swapchain);
	}

	VulkanRenderpass::~VulkanRenderpass()
	{
		vkDestroyRenderPass(m_Context->GetLogicalDevice(), m_Renderpass, nullptr);

		DestroyFramebuffers();
	}

	void VulkanRenderpass::Begin(const glm::vec2& size, std::shared_ptr<CommandBuffer> cmd, const std::vector<ClearValue>& clearValues, uint32_t swapchainImageIndex) const
	{
		GA_PROFILE_SCOPE();

		std::shared_ptr<VulkanCommandBuffer> vulkanCommandBuffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(cmd);

		std::vector<VkClearValue> vulkanClearValues;
		for (const ClearValue& value : clearValues)
		{
			VkClearDepthStencilValue depthValue;
			depthValue.depth = value.Depth;
			depthValue.stencil = value.Stencil;

			VkClearValue vulkanClearValue;
			vulkanClearValue.color = { value.ClearColor.r, value.ClearColor.g, value.ClearColor.b, value.ClearColor.a };
			vulkanClearValue.depthStencil = depthValue;

			vulkanClearValues.push_back(vulkanClearValue);
		}

		VkRenderPassBeginInfo rpInfo = {};
		rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpInfo.pNext = nullptr;
		rpInfo.renderPass = m_Renderpass;
		rpInfo.renderArea.offset.x = 0;
		rpInfo.renderArea.offset.y = 0;
		rpInfo.renderArea.extent = { (uint32_t)size.x, (uint32_t)size.y };
		rpInfo.framebuffer = m_Framebuffers[swapchainImageIndex];
		
		rpInfo.clearValueCount = (uint32_t)vulkanClearValues.size();
		rpInfo.pClearValues = vulkanClearValues.data();

		vkCmdBeginRenderPass(vulkanCommandBuffer->GetInternal(), &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VulkanRenderpass::End(std::shared_ptr<CommandBuffer> cmd) const
	{
		GA_PROFILE_SCOPE();

		std::shared_ptr<VulkanCommandBuffer> vulkanCommandBuffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(cmd);
		vkCmdEndRenderPass(vulkanCommandBuffer->GetInternal());
	}

	void VulkanRenderpass::Recreate(const Specification& spec)
	{
		DestroyFramebuffers();
		CreateFramebuffers(spec);
	}

	void VulkanRenderpass::DestroyFramebuffers()
	{
		for (VkFramebuffer& framebuffer : m_Framebuffers)
			vkDestroyFramebuffer(m_Context->GetLogicalDevice(), framebuffer, nullptr);
	}

	void VulkanRenderpass::InitRenderpass(const Specification& spec)
	{
		GA_PROFILE_SCOPE();

		std::vector<VkAttachmentDescription> attachments;
		std::vector<VkAttachmentReference> colorAttachmentRefs;
		VkAttachmentReference depthAttachment;

		std::vector<VkSubpassDependency> dependencies;

		int attachmentIndex = 0;

		for (auto& colorOutput : spec.ColorOutputs)
		{
			std::shared_ptr<VulkanImage> vulkanImage = std::dynamic_pointer_cast<VulkanImage>(colorOutput.Images[0]);

			VkAttachmentDescription attachment = {};
			attachment.format = vulkanImage->GetFormat();
			attachment.samples = vulkanImage->GetSamples();
			attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // have to change below when adding more complciated logic
			attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference attachmentRef = {};
			attachmentRef.attachment = attachmentIndex++;
			attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDependency dependency = {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			attachments.push_back(attachment);
			colorAttachmentRefs.push_back(attachmentRef);
			dependencies.push_back(dependency);
		}

		for (auto& depthOutput : spec.DepthStencilOutput)
		{
			std::shared_ptr<VulkanImage> vulkanImage = std::dynamic_pointer_cast<VulkanImage>(depthOutput.Images[0]);

			VkAttachmentDescription attachment = {};
			attachment.format = vulkanImage->GetFormat();
			attachment.samples = vulkanImage->GetSamples();
			attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // have to change below when adding more complciated logic
			attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentReference attachmentRef = {};
			attachmentRef.attachment = attachmentIndex++;
			attachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkSubpassDependency dependency = {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			attachments.push_back(attachment);
			depthAttachment = attachmentRef;
			dependencies.push_back(dependency);
		}

		VkSubpassDescription subpass = {}; // need to rewrite this
		subpass.colorAttachmentCount = (uint32_t)colorAttachmentRefs.size();
		subpass.pColorAttachments = colorAttachmentRefs.data();
		subpass.pDepthStencilAttachment = &depthAttachment;
		subpass.pipelineBindPoint = m_Bindpoint;

		VkRenderPassCreateInfo renderpassInfo = {};
		renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderpassInfo.attachmentCount = (uint32_t)attachments.size();
		renderpassInfo.pAttachments = attachments.data();
		renderpassInfo.subpassCount = 1;
		renderpassInfo.pSubpasses = &subpass;
		renderpassInfo.dependencyCount = (uint32_t)dependencies.size();
		renderpassInfo.pDependencies = dependencies.data();

		VK_CHECK(vkCreateRenderPass(m_Context->GetLogicalDevice(), &renderpassInfo, nullptr, &m_Renderpass));
	}

	void VulkanRenderpass::CreateFramebuffers(const Specification& spec)
	{
		GA_PROFILE_SCOPE();

		VkFramebufferCreateInfo fbInfo = {};
		fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbInfo.pNext = nullptr;
		fbInfo.renderPass = m_Renderpass;
		fbInfo.attachmentCount = 1;
		fbInfo.width = (uint32_t)spec.Size.x;
		fbInfo.height = (uint32_t)spec.Size.y;
		fbInfo.layers = 1;

		m_Framebuffers.resize(spec.FramebufferCount);
		for (uint32_t i = 0; i < spec.FramebufferCount; i++)
		{
			std::vector<VkImageView> attachments;

			for (auto& attachment : spec.ColorOutputs)
			{
				auto& images = attachment.Images;

				if (images.size() == 1) attachments.push_back(std::dynamic_pointer_cast<VulkanImage>(images[0])->GetView());
				else attachments.push_back(std::dynamic_pointer_cast<VulkanImage>(images[i])->GetView());
			}

			for (auto& attachment : spec.DepthStencilOutput)
			{
				auto& images = attachment.Images;

				if (images.size() == 1) attachments.push_back(std::dynamic_pointer_cast<VulkanImage>(images[0])->GetView());
				else attachments.push_back(std::dynamic_pointer_cast<VulkanImage>(images[i])->GetView());
			}

			fbInfo.attachmentCount = (uint32_t)attachments.size();
			fbInfo.pAttachments = attachments.data();
			
			VK_CHECK(vkCreateFramebuffer(m_Context->GetLogicalDevice(), &fbInfo, nullptr, &m_Framebuffers[i]));
		}
	}

}