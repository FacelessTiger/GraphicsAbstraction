#include "VulkanCommandBuffer.h"

#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanSwapchain.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanShader.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanImage.h>
#include <Platform/GraphicsAPI/Vulkan/InternalManagers/VulkanUtils.h>
#include <GraphicsAbstraction/Debug/Instrumentor.h>

namespace GraphicsAbstraction {

	namespace Utils {

		VkCompareOp GACompareOpToVulkan(CompareOperation op)
		{
			switch (op)
			{
				case CompareOperation::GreaterEqual:	return VK_COMPARE_OP_GREATER_OR_EQUAL;
				case CompareOperation::LesserEqual:		return VK_COMPARE_OP_LESS_OR_EQUAL;
			}

			GA_CORE_ASSERT(false, "Unknown compare operation!");
			return (VkCompareOp)0;
		}

		VkBlendFactor GABlendToVulkan(Blend blend)
		{
			switch (blend)
			{
				case Blend::Zero:				return VK_BLEND_FACTOR_ZERO;
				case Blend::One:				return VK_BLEND_FACTOR_ONE;
				case Blend::SrcAlpha:			return VK_BLEND_FACTOR_SRC_ALPHA;
				case Blend::DstAlpha:			return VK_BLEND_FACTOR_DST_ALPHA;
				case Blend::OneMinusSrcAlpha:	return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			}

			GA_CORE_ASSERT(false, "Unknown blend factor!");
			return (VkBlendFactor)0;
		}

		VkBlendOp GABlendOpToVulkan(BlendOp blendOp)
		{
			switch (blendOp)
			{
				case BlendOp::Add: return VK_BLEND_OP_ADD;
			}

			GA_CORE_ASSERT(false, "Unknown blend operation!");
			return (VkBlendOp)0;
		}

	}

	VulkanCommandBuffer::VulkanCommandBuffer(VkCommandBuffer buffer)
		: m_Context(VulkanContext::GetReference()), CommandBuffer(buffer)
	{
		
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{

	}

	void VulkanCommandBuffer::Clear(const Ref<Image>& image, const glm::vec4& color)
	{
		auto& vulkanImage = (VulkanImage&)(*image);
		vulkanImage.TransitionLayout(CommandBuffer, VK_IMAGE_LAYOUT_GENERAL);

		VkClearColorValue clearValue = { { color.x, color.y, color.z, color.w } };
		VkImageSubresourceRange clearRange = Utils::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

		vkCmdClearColorImage(CommandBuffer, vulkanImage.Image.Image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
	}

	void VulkanCommandBuffer::Present(const Ref<Swapchain>& swapchain)
	{
		auto& vulkanSwapchain = (VulkanSwapchain&)(*swapchain);
		auto image = vulkanSwapchain.Images[vulkanSwapchain.ImageIndex];

		image->TransitionLayout(CommandBuffer, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}

	void VulkanCommandBuffer::Dispatch(uint32_t workX, uint32_t workY, uint32_t workZ)
	{
		if (!m_Context->ShaderObjectSupported && m_ComputePipelineStateChanged)
		{
			vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_Context->PipelineManager->GetComputePipeline(m_ComputePipelineKey));
			m_ComputePipelineStateChanged = false;
		}

		vkCmdDispatch(CommandBuffer, workX, workY, workZ);
	}

	void VulkanCommandBuffer::CopyToBuffer(const Ref<Buffer>& src, const Ref<Buffer>& dst, uint32_t size, uint32_t srcOffset, uint32_t dstOffset)
	{
		auto& vulkanSrc = (VulkanBuffer&)(*src);
		auto& vulkanDst = (VulkanBuffer&)(*dst);

		VkBufferCopy copy = {
			.srcOffset = srcOffset,
			.dstOffset = dstOffset,
			.size = size
		};
		vkCmdCopyBuffer(CommandBuffer, vulkanSrc.Buffer.Buffer, vulkanDst.Buffer.Buffer, 1, &copy);
	}

	void VulkanCommandBuffer::CopyToImage(const Ref<Buffer>& src, const Ref<Image>& dst, uint32_t srcOffset)
	{
		auto& vulkanSrc = (VulkanBuffer&)(*src);
		auto& vulkanDst = (VulkanImage&)(*dst);

		vulkanDst.TransitionLayout(CommandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkBufferImageCopy copy = {
			.bufferOffset = srcOffset,
			.bufferRowLength = 0,
			.bufferImageHeight = 0
		};

		copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy.imageSubresource.mipLevel = 0;
		copy.imageSubresource.baseArrayLayer = 0;
		copy.imageSubresource.layerCount = 1;
		copy.imageExtent = { vulkanDst.Width, vulkanDst.Height, 1 };

		vkCmdCopyBufferToImage(CommandBuffer, vulkanSrc.Buffer.Buffer, vulkanDst.Image.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
		vulkanDst.TransitionLayout(CommandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void VulkanCommandBuffer::BeginRendering(const glm::vec2& region, const std::vector<Ref<Image>>& colorAttachments, const Ref<Image>& depthAttachment)
	{
		GA_PROFILE_SCOPE();

		if (m_Context->DynamicRenderingSupported)
		{
			std::vector<VkRenderingAttachmentInfo> vulkanColorAttachments;
			VkRenderingAttachmentInfo vulkanDepthAttachment = {};

			for (int i = 0; i < colorAttachments.size(); i++)
			{
				auto& vulkanImage = (VulkanImage&)(*colorAttachments[i]);
				vulkanImage.TransitionLayout(CommandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
				m_GraphicsPipelineKey.ColorAttachments[i] = vulkanImage.Format;

				vulkanColorAttachments.push_back({
					.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
					.imageView = vulkanImage.View,
					.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE
				});
			}

			m_GraphicsPipelineKey.DepthAttachment = VK_FORMAT_UNDEFINED;
			if (depthAttachment)
			{
				auto& vulkanImage = (VulkanImage&)(*depthAttachment);
				vulkanImage.TransitionLayout(CommandBuffer, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
				m_GraphicsPipelineKey.DepthAttachment = vulkanImage.Format;

				vulkanDepthAttachment = {
					.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
					.imageView = vulkanImage.View,
					.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE
				};
				vulkanDepthAttachment.clearValue.depthStencil.depth = 0.0f;
			}

			VkExtent2D extent = {
				.width = (uint32_t)region.x,
				.height = (uint32_t)region.y
			};

			VkRect2D rect = {
				.extent = extent
			};

			VkRenderingInfo renderInfo = {
				.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
				.renderArea = rect,
				.layerCount = 1,
				.colorAttachmentCount = (uint32_t)vulkanColorAttachments.size(),
				.pColorAttachments = vulkanColorAttachments.data(),
				.pDepthAttachment = depthAttachment ? &vulkanDepthAttachment : nullptr
			};

			m_GraphicsPipelineStateChanged = true;
			m_Context->vkCmdBeginRenderingKHR(CommandBuffer, &renderInfo);
		}
		else
		{
			VulkanRenderInfoKey key;
			key.Width = (uint32_t)region.x;
			key.Height = (uint32_t)region.y;

			std::vector<VkImageView> attachmentViews;
			attachmentViews.reserve(colorAttachments.size());

			if (depthAttachment)
			{
				auto& vulkanImage = (VulkanImage&)(*depthAttachment);
				key.DepthAttachment = { vulkanImage.Format, vulkanImage.Layout, vulkanImage.Usage };

				vulkanImage.Layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				attachmentViews.push_back(vulkanImage.View);
			}

			for (int i = 0; i < colorAttachments.size(); i++)
			{
				auto& vulkanImage = (VulkanImage&)(*colorAttachments[i]);
				key.ColorAttachments[i] = { vulkanImage.Format, vulkanImage.Layout, vulkanImage.Usage };

				vulkanImage.Layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				attachmentViews.push_back(vulkanImage.View);
			}

			VulkanRenderInfo info = m_Context->RenderInfoManager->GetRenderInfo(key);
			m_GraphicsPipelineKey.Renderpass = info.renderpass;

			VkRenderPassAttachmentBeginInfo attachmentInfo = {
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO,
				.attachmentCount = (uint32_t)attachmentViews.size(),
				.pAttachments = attachmentViews.data()
			};
			
			VkClearValue depthClear = {};
			depthClear.depthStencil.depth = 0.0f;

			VkRenderPassBeginInfo beginInfo = {
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				.pNext = &attachmentInfo,
				.renderPass = info.renderpass,
				.framebuffer = info.framebuffer,
				.renderArea = { {}, { (uint32_t)region.x, (uint32_t)region.y } },
				.clearValueCount = 1,
				.pClearValues = &depthClear
			};

			m_GraphicsPipelineStateChanged = true;
			vkCmdBeginRenderPass(CommandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
		}
	}

	void VulkanCommandBuffer::EndRendering()
	{
		if (m_Context->DynamicRenderingSupported) m_Context->vkCmdEndRenderingKHR(CommandBuffer);
		else vkCmdEndRenderPass(CommandBuffer);
	}

	void VulkanCommandBuffer::BindShaders(const std::vector<Ref<Shader>> shaderStages)
	{
		GA_PROFILE_SCOPE();

		std::vector<VkShaderStageFlagBits> stages;
		std::vector<VkShaderEXT> shaders;
		stages.reserve(shaderStages.size());
		shaders.reserve(shaderStages.size());

		for (auto& shaderStage : shaderStages)
		{
			auto& vulkanShaderStage = (VulkanShader&)(*shaderStage);
			if (m_Context->ShaderObjectSupported)
			{
				stages.push_back(vulkanShaderStage.Stage);
				shaders.push_back(vulkanShaderStage.ShaderObject);
			}
			else
			{
				switch (vulkanShaderStage.Stage)
				{
					case VK_SHADER_STAGE_VERTEX_BIT:
					{
						m_GraphicsPipelineKey.Shaders[0] = vulkanShaderStage.ID;
						m_GraphicsPipelineStateChanged = true;
						break;
					}
					case VK_SHADER_STAGE_FRAGMENT_BIT:
					{
						m_GraphicsPipelineKey.Shaders[4] = vulkanShaderStage.ID;
						m_GraphicsPipelineStateChanged = true;
						break;
					}
					case VK_SHADER_STAGE_COMPUTE_BIT:
					{
						m_ComputePipelineKey.Shader = vulkanShaderStage.ID;
						m_ComputePipelineStateChanged = true;
						break;
					}
				}
			}
		}

		if (m_Context->ShaderObjectSupported) m_Context->vkCmdBindShadersEXT(CommandBuffer, (uint32_t)shaders.size(), stages.data(), shaders.data());
	}

	void VulkanCommandBuffer::BindIndexBuffer(const Ref<Buffer>& buffer)
	{
		auto& vulkanBuffer = (VulkanBuffer&)(*buffer);
		vkCmdBindIndexBuffer(CommandBuffer, vulkanBuffer.Buffer.Buffer, 0, VK_INDEX_TYPE_UINT16);
	}

	void VulkanCommandBuffer::PushConstant(const void* data, uint32_t size, uint32_t offset)
	{
		vkCmdPushConstants(CommandBuffer, m_Context->BindlessPipelineLayout, VK_SHADER_STAGE_ALL, offset, size, data);
	}

	void VulkanCommandBuffer::SetViewport(const glm::vec2& size)
	{
		float yOffset = (size.y < 0) ? 0 : size.y;
		VkViewport viewport = {
			.x = 0,
			.y = yOffset,
			.width = size.x,
			.height = -size.y,
			.minDepth = 0.0f,
			.maxDepth = 1.0f
		};

		if (m_Context->DynamicStateSupported) m_Context->vkCmdSetViewportWithCountEXT(CommandBuffer, 1, &viewport);
		else vkCmdSetViewport(CommandBuffer, 0, 1, &viewport);
	}

	void VulkanCommandBuffer::SetScissor(const glm::vec2& size, const glm::vec2& offset)
	{
		VkOffset2D offset2D = {
			.x = (int32_t)offset.x,
			.y = (int32_t)offset.y
		};

		VkExtent2D extent = {
			.width = (uint32_t)size.x,
			.height = (uint32_t)size.y
		};

		VkRect2D scissor = {
			.offset = offset2D,
			.extent = extent
		};

		if (m_Context->DynamicStateSupported) m_Context->vkCmdSetScissorWithCountEXT(CommandBuffer, 1, &scissor);
		else vkCmdSetScissor(CommandBuffer, 0, 1, &scissor);
	}

	void VulkanCommandBuffer::SetDepthTest(bool testEnabled, bool writeEnabled, CompareOperation op)
	{
		vkCmdSetDepthBounds(CommandBuffer, 0.0f, 1.0f);
		if (m_Context->DynamicStateSupported)
		{
			m_Context->vkCmdSetDepthTestEnableEXT(CommandBuffer, testEnabled);
			m_Context->vkCmdSetDepthWriteEnableEXT(CommandBuffer, writeEnabled);
			m_Context->vkCmdSetDepthCompareOpEXT(CommandBuffer, Utils::GACompareOpToVulkan(op));
			m_DepthEnableSet = true;
		}
		else
		{
			m_GraphicsPipelineKey.DepthTestEnable = testEnabled;
			m_GraphicsPipelineKey.DepthWriteEnable = writeEnabled;
			m_GraphicsPipelineKey.DepthCompareOp = Utils::GACompareOpToVulkan(op);
			m_GraphicsPipelineStateChanged = true;
		}

	}

	void VulkanCommandBuffer::EnableColorBlend(Blend srcBlend, Blend dstBlend, BlendOp blendOp, Blend srcBlendAlpha, Blend dstBlendAlpha, BlendOp blendAlpha)
	{
		SetColorBlend(true,		Utils::GABlendToVulkan(srcBlend), Utils::GABlendToVulkan(dstBlend), Utils::GABlendOpToVulkan(blendOp), 
								Utils::GABlendToVulkan(srcBlendAlpha), Utils::GABlendToVulkan(dstBlendAlpha), Utils::GABlendOpToVulkan(blendAlpha));
	}

	void VulkanCommandBuffer::DisableColorBlend()
	{
		SetColorBlend(false,	VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
								VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);
	}

	void VulkanCommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
	{
		SetDynamicState();
		vkCmdDraw(CommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void VulkanCommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
	{
		SetDynamicState();
		vkCmdDrawIndexed(CommandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void VulkanCommandBuffer::SetColorBlend(bool enabled, VkBlendFactor srcBlend, VkBlendFactor dstBlend, VkBlendOp blendOp, VkBlendFactor srcBlendAlpha, VkBlendFactor dstBlendAlpha, VkBlendOp blendAlpha)
	{
		if (m_Context->DynamicState3Supported)
		{
			VkBool32 enable = enabled;
			VkColorBlendEquationEXT equation = {
				.srcColorBlendFactor = srcBlend,
				.dstColorBlendFactor = dstBlend,
				.colorBlendOp = blendOp,
				.srcAlphaBlendFactor = srcBlendAlpha,
				.dstAlphaBlendFactor = dstBlendAlpha,
				.alphaBlendOp = blendAlpha
			};

			m_Context->vkCmdSetColorBlendEnableEXT(CommandBuffer, 0, 1, &enable);
			m_Context->vkCmdSetColorBlendEquationEXT(CommandBuffer, 0, 1, &equation);
			m_ColorBlendSet = true;
		}
		else
		{
			m_GraphicsPipelineKey.BlendEnable = enabled;
			m_GraphicsPipelineKey.SrcBlend = srcBlend;
			m_GraphicsPipelineKey.DstBlend = dstBlend;
			m_GraphicsPipelineKey.BlendOp = blendOp;
			m_GraphicsPipelineKey.SrcBlendAlpha = srcBlendAlpha;
			m_GraphicsPipelineKey.DstBlendAlpha = dstBlendAlpha;
			m_GraphicsPipelineKey.BlendOpAlpha = blendAlpha;
			m_GraphicsPipelineStateChanged = true;
		}
	}

	void VulkanCommandBuffer::SetDynamicState()
	{
		GA_PROFILE_SCOPE();

		if (!m_Context->ShaderObjectSupported && m_GraphicsPipelineStateChanged)
		{
			vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Context->PipelineManager->GetGraphicsPipeline(m_GraphicsPipelineKey));
			m_GraphicsPipelineStateChanged = false;
		}
		if (m_DefaultDynamicStateSet) return;

		VkSampleMask mask = ~0;
		VkBool32 colorBlendEnable = false;
		VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		VkColorBlendEquationEXT colorBlendEquation = {
			.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO,
			.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
			.colorBlendOp = VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
			.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
			.alphaBlendOp = VK_BLEND_OP_ADD
		};

		vkCmdSetLineWidth(CommandBuffer, 1.0f);

		if (m_Context->DynamicStateSupported)
		{
			m_Context->vkCmdSetCullModeEXT(CommandBuffer, VK_CULL_MODE_NONE);
			m_Context->vkCmdSetFrontFaceEXT(CommandBuffer, VK_FRONT_FACE_CLOCKWISE);
			m_Context->vkCmdSetPrimitiveTopologyEXT(CommandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

			if (!m_DepthEnableSet)
			{
				m_Context->vkCmdSetDepthTestEnableEXT(CommandBuffer, false);
				m_Context->vkCmdSetDepthWriteEnableEXT(CommandBuffer, false);
			}

			m_Context->vkCmdSetDepthBoundsTestEnableEXT(CommandBuffer, false);
			m_Context->vkCmdSetStencilTestEnableEXT(CommandBuffer, false);
			m_Context->vkCmdSetStencilOpEXT(CommandBuffer, VK_STENCIL_FACE_FRONT_BIT, {}, {}, {}, {});
		}

		if (m_Context->DynamicState2Supported)
		{
			m_Context->vkCmdSetRasterizerDiscardEnableEXT(CommandBuffer, false);
			m_Context->vkCmdSetDepthBiasEnableEXT(CommandBuffer, false);
			m_Context->vkCmdSetPrimitiveRestartEnableEXT(CommandBuffer, false);
		}

		if (m_Context->DynamicState3Supported)
		{
			m_Context->vkCmdSetPolygonModeEXT(CommandBuffer, VK_POLYGON_MODE_FILL); 
			m_Context->vkCmdSetRasterizationSamplesEXT(CommandBuffer, VK_SAMPLE_COUNT_1_BIT); 
			m_Context->vkCmdSetSampleMaskEXT(CommandBuffer, VK_SAMPLE_COUNT_1_BIT, &mask);
			m_Context->vkCmdSetAlphaToCoverageEnableEXT(CommandBuffer, false);

			if (!m_ColorBlendSet)
			{
				m_Context->vkCmdSetColorBlendEnableEXT(CommandBuffer, 0, 1, &colorBlendEnable);
				m_Context->vkCmdSetColorBlendEquationEXT(CommandBuffer, 0, 1, &colorBlendEquation);
			}

			m_Context->vkCmdSetColorWriteMaskEXT(CommandBuffer, 0, 1, &colorWriteMask);
		}

		if (m_Context->ShaderObjectSupported) m_Context->vkCmdSetVertexInputEXT(CommandBuffer, 0, nullptr, 0, nullptr);

		m_DefaultDynamicStateSet = true;
	}

}