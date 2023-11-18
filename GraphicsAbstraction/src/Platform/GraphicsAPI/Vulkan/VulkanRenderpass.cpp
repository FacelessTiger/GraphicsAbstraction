#include "VulkanRenderpass.h"

#include <Platform/GraphicsAPI/Vulkan/VulkanContext.h>
#include <Platform/GraphicsAPI/Vulkan/VulkanSwapchain.h>
#include <Platform/GraphicsAPI/Vulkan/VulkanCommandBuffer.h>

#include <GraphicsAbstraction/Core/Core.h>
#include <GraphicsAbstraction/Debug/Instrumentor.h>

namespace GraphicsAbstraction {

	namespace Utils {

		VkAttachmentLoadOp GALoadOperationToVulkan(Renderpass::LoadOperation loadOperation)
		{
			switch (loadOperation)
			{
				case Renderpass::LoadOperation::None:					return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				case Renderpass::LoadOperation::Clear:					return VK_ATTACHMENT_LOAD_OP_CLEAR;
			}

			GA_CORE_ERROR("Unknown load operation for renderpass, {0}. Defaulting to none", (int)loadOperation);
			return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		}

		VkAttachmentStoreOp GAStoreOperationToVulkan(Renderpass::StoreOperation storeOperation)
		{
			switch (storeOperation)
			{
				case Renderpass::StoreOperation::None:					return VK_ATTACHMENT_STORE_OP_DONT_CARE;
				case Renderpass::StoreOperation::Store:					return VK_ATTACHMENT_STORE_OP_STORE;
			}

			GA_CORE_ERROR("Unknown store operation for renderpass, {0}. Defaulting to none", (int)storeOperation);
			return VK_ATTACHMENT_STORE_OP_DONT_CARE;
		}

		VkImageLayout GAImageLayoutToVulkan(Renderpass::ImageLayout imageLayout)
		{
			switch (imageLayout)
			{
				case Renderpass::ImageLayout::None:						return VK_IMAGE_LAYOUT_UNDEFINED;
				case Renderpass::ImageLayout::ColorAttachmentOptimal:	return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				case Renderpass::ImageLayout::PresentSource:			return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			}

			GA_CORE_ERROR("Unknown image layout for renderpass, {0}. Defaulting to none", (int)imageLayout);
			return VK_IMAGE_LAYOUT_UNDEFINED;
		}

		static VkPipelineBindPoint GAPipelineBindpointToVulkan(Renderpass::PipelineBindpoint pipelineBindpoint)
		{
			switch (pipelineBindpoint)
			{
				case Renderpass::PipelineBindpoint::Graphics:			return VK_PIPELINE_BIND_POINT_GRAPHICS;
			}

			GA_CORE_ERROR("Unknown pipeline bindpoint for renderpass, {0}. Defaulting to Graphics", (int)pipelineBindpoint);
			return VK_PIPELINE_BIND_POINT_GRAPHICS;
		}

	}

	VulkanRenderpass::VulkanRenderpass(const Specification& spec, std::shared_ptr<GraphicsContext> context, std::shared_ptr<Swapchain> swapchain)
	{
		GA_PROFILE_SCOPE();

		m_Context = std::dynamic_pointer_cast<VulkanContext>(context);
		std::shared_ptr<VulkanSwapchain> vulkanSwapchain = std::dynamic_pointer_cast<VulkanSwapchain>(swapchain);

		InitRenderpass(spec, vulkanSwapchain);
		Recreate(swapchain);
	}

	VulkanRenderpass::~VulkanRenderpass()
	{
		vkDestroyRenderPass(m_Context->GetLogicalDevice(), m_Renderpass, nullptr);

		DestroyFramebuffers();
	}

	void VulkanRenderpass::Begin(std::shared_ptr<Swapchain> swapchain, std::shared_ptr<CommandBuffer> cmd, const glm::vec4& clearColor, uint32_t swapchainImageIndex) const
	{
		GA_PROFILE_SCOPE();

		std::shared_ptr<VulkanSwapchain> vulkanSwapchain = std::dynamic_pointer_cast<VulkanSwapchain>(swapchain);
		std::shared_ptr<VulkanCommandBuffer> vulkanCommandBuffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(cmd);

		VkClearValue clearValue;
		clearValue.color = { { clearColor.x, clearColor.y, clearColor.z, clearColor.w } };

		VkRenderPassBeginInfo rpInfo = {};
		rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpInfo.pNext = nullptr;
		rpInfo.renderPass = m_Renderpass;
		rpInfo.renderArea.offset.x = 0;
		rpInfo.renderArea.offset.y = 0;
		rpInfo.renderArea.extent = { vulkanSwapchain->GetWidth(), vulkanSwapchain->GetHeight() };
		rpInfo.framebuffer = m_Framebuffers[swapchainImageIndex];
		
		rpInfo.clearValueCount = 1;
		rpInfo.pClearValues = &clearValue;

		vkCmdBeginRenderPass(vulkanCommandBuffer->GetInternal(), &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VulkanRenderpass::End(std::shared_ptr<CommandBuffer> cmd) const
	{
		GA_PROFILE_SCOPE();

		std::shared_ptr<VulkanCommandBuffer> vulkanCommandBuffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(cmd);
		vkCmdEndRenderPass(vulkanCommandBuffer->GetInternal());
	}

	void VulkanRenderpass::Recreate(std::shared_ptr<Swapchain> swapchain)
	{
		DestroyFramebuffers();
		CreateFramebuffers(swapchain);
	}

	void VulkanRenderpass::DestroyFramebuffers()
	{
		for (VkFramebuffer& framebuffer : m_Framebuffers)
			vkDestroyFramebuffer(m_Context->GetLogicalDevice(), framebuffer, nullptr);
	}

	void VulkanRenderpass::InitRenderpass(const Specification& spec, std::shared_ptr<VulkanSwapchain> swapchain)
	{
		GA_PROFILE_SCOPE();

		std::vector<VkAttachmentDescription> vulkanAttachments;
		vulkanAttachments.reserve(spec.Attachments.size());

		for (const Attachment& attachment : spec.Attachments)
		{
			VkAttachmentDescription vulkanAttachment = {};
			vulkanAttachment.format = swapchain->GetImageFormat();
			vulkanAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

			vulkanAttachment.loadOp = Utils::GALoadOperationToVulkan(attachment.LoadOperation);
			vulkanAttachment.storeOp = Utils::GAStoreOperationToVulkan(attachment.StoreOperation);

			vulkanAttachment.stencilLoadOp = Utils::GALoadOperationToVulkan(attachment.StencilLoadOperation);
			vulkanAttachment.stencilStoreOp = Utils::GAStoreOperationToVulkan(attachment.StencilStoreOperation);

			vulkanAttachment.initialLayout = Utils::GAImageLayoutToVulkan(attachment.InitialImageLayout);
			vulkanAttachment.finalLayout = Utils::GAImageLayoutToVulkan(attachment.FinalImageLayout);

			vulkanAttachments.emplace_back(vulkanAttachment);
		}

		std::vector<VkSubpassDescription> vulkanSubpasses;
		std::vector<std::vector<VkAttachmentReference>> vulkanColorAttachmentsList(spec.Subpasses.size());
		vulkanSubpasses.reserve(spec.Subpasses.size());

		for (int i = 0; i < spec.Subpasses.size(); i++)
		{
			const SubpassSpecification& subpass = spec.Subpasses[i];

			std::vector<VkAttachmentReference>& vulkanColorAttachments = vulkanColorAttachmentsList[i];
			vulkanColorAttachments.reserve(subpass.ColorAttachments.size());

			for (const AttachmentReference& colorAttachement : subpass.ColorAttachments)
			{
				VkAttachmentReference vulkanColorAttachement = {};
				vulkanColorAttachement.attachment = colorAttachement.AttachmentIndex;
				vulkanColorAttachement.layout = Utils::GAImageLayoutToVulkan(colorAttachement.ImageLayout);

				vulkanColorAttachments.emplace_back(vulkanColorAttachement);
			}

			VkSubpassDescription vulkanSubpass = {};
			vulkanSubpass.pipelineBindPoint = Utils::GAPipelineBindpointToVulkan(subpass.Bindpoint);

			if (vulkanColorAttachments.size())
			{
				vulkanSubpass.colorAttachmentCount = (uint32_t)vulkanColorAttachments.size();
				vulkanSubpass.pColorAttachments = vulkanColorAttachments.data();
			}

			vulkanSubpasses.emplace_back(vulkanSubpass);
		}

		VkRenderPassCreateInfo renderpassInfo = {};
		renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderpassInfo.attachmentCount = (uint32_t)vulkanAttachments.size();
		renderpassInfo.pAttachments = vulkanAttachments.data();
		renderpassInfo.subpassCount = (uint32_t)vulkanSubpasses.size();
		renderpassInfo.pSubpasses = vulkanSubpasses.data();

		VK_CHECK(vkCreateRenderPass(m_Context->GetLogicalDevice(), &renderpassInfo, nullptr, &m_Renderpass));
	}

	void VulkanRenderpass::CreateFramebuffers(std::shared_ptr<Swapchain> swapchain)
	{
		GA_PROFILE_SCOPE();

		std::shared_ptr<VulkanSwapchain> vulkanSwapchain = std::dynamic_pointer_cast<VulkanSwapchain>(swapchain);

		VkFramebufferCreateInfo fbInfo = {};
		fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbInfo.pNext = nullptr;

		fbInfo.renderPass = m_Renderpass;
		fbInfo.attachmentCount = 1;
		fbInfo.width = vulkanSwapchain->GetWidth();
		fbInfo.height = vulkanSwapchain->GetHeight();
		fbInfo.layers = 1;

		m_Framebuffers.resize(vulkanSwapchain->GetImageCount());
		const auto& imageViews = vulkanSwapchain->GetImageViews();

		for (uint32_t i = 0; i < vulkanSwapchain->GetImageCount(); i++)
		{
			fbInfo.pAttachments = &imageViews[i];
			VK_CHECK(vkCreateFramebuffer(m_Context->GetLogicalDevice(), &fbInfo, nullptr, &m_Framebuffers[i]));
		}
	}

}