#include "VulkanCommandList.h"

#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanSwapchain.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanShader.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanImage.h>
#include <Platform/GraphicsAPI/Vulkan/InternalManagers/VulkanUtils.h>
#include <GraphicsAbstraction/Debug/Instrumentor.h>

namespace GraphicsAbstraction {

	VulkanCommandList::VulkanCommandList(VkCommandBuffer buffer)
		: m_Context(VulkanContext::GetReference()), CommandBuffer(buffer)
	{
		
	}

	VulkanCommandList::~VulkanCommandList()
	{

	}

	void VulkanCommandList::Clear(const Ref<Image>& image, const glm::vec4& color)
	{
		auto& vulkanImage = (VulkanImage&)(*image);
		vulkanImage.TransitionLayout(CommandBuffer, VK_IMAGE_LAYOUT_GENERAL);

		VkClearColorValue clearValue = { { color.x, color.y, color.z, color.w } };
		VkImageSubresourceRange clearRange = Utils::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

		vkCmdClearColorImage(CommandBuffer, vulkanImage.Image.Image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
	}

	void VulkanCommandList::Present(const Ref<Swapchain>& swapchain)
	{
		auto& vulkanSwapchain = (VulkanSwapchain&)(*swapchain);
		auto image = vulkanSwapchain.Images[vulkanSwapchain.ImageIndex];

		image->TransitionLayout(CommandBuffer, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}

	void VulkanCommandList::CopyBufferRegion(const Ref<Buffer>& src, const Ref<Buffer>& dst, uint32_t size, uint32_t srcOffset, uint32_t dstOffset)
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

	void VulkanCommandList::CopyToImage(const Ref<Buffer>& src, const Ref<Image>& dst, uint32_t srcOffset)
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

	void VulkanCommandList::CopyToImage(const Ref<Image>& src, const Ref<Image>& dst)
	{
		auto& vulkanSrc = (VulkanImage&)(*src);
		auto& vulkanDst = (VulkanImage&)(*dst);

		vulkanSrc.TransitionLayout(CommandBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		vulkanDst.TransitionLayout(CommandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkImageCopy copy = {
			.extent = { vulkanSrc.Width, vulkanSrc.Height, 1 }
		};
		copy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy.srcSubresource.layerCount = 1;

		copy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy.dstSubresource.layerCount = 1;

		vkCmdCopyImage(CommandBuffer, vulkanSrc.Image.Image, vulkanSrc.Layout, vulkanDst.Image.Image, vulkanDst.Layout, 1, &copy);
	}

	void VulkanCommandList::RWResourceBarrier(const Ref<Image>& resource)
	{
		auto& vulkanImage = (VulkanImage&)(*resource);
		if (vulkanImage.Layout == VK_IMAGE_LAYOUT_UNDEFINED) return;
		
		VkImageMemoryBarrier barrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
			.oldLayout = vulkanImage.Layout,
			.newLayout = vulkanImage.Layout,

			.image = vulkanImage.Image.Image,
			.subresourceRange = Utils::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT)
		};
		vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	void VulkanCommandList::RWResourceBarrier(const Ref<Buffer>& resource)
	{
		auto& vulkanBuffer = (VulkanBuffer&)(*resource);

		VkBufferMemoryBarrier barrier = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
			.buffer = vulkanBuffer.Buffer.Buffer,
			.size = VK_WHOLE_SIZE
		};
		vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 1, &barrier, 0, nullptr);
	}

	void VulkanCommandList::BeginRendering(const glm::vec2& region, const std::vector<Ref<Image>>& colorAttachments, const Ref<Image>& depthAttachment)
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

			m_GraphicsPipelineKey.DepthAttachment = ImageFormat::Unknown;
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

	void VulkanCommandList::EndRendering()
	{
		if (m_Context->DynamicRenderingSupported) m_Context->vkCmdEndRenderingKHR(CommandBuffer);
		else vkCmdEndRenderPass(CommandBuffer);
	}

	void VulkanCommandList::BindShaders(const std::vector<Ref<Shader>> shaderStages)
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

	void VulkanCommandList::BindIndexBuffer(const Ref<Buffer>& buffer)
	{
		auto& vulkanBuffer = (VulkanBuffer&)(*buffer);
		vkCmdBindIndexBuffer(CommandBuffer, vulkanBuffer.Buffer.Buffer, 0, VK_INDEX_TYPE_UINT16);
	}

	void VulkanCommandList::PushConstant(const void* data, uint32_t size, uint32_t offset)
	{
		vkCmdPushConstants(CommandBuffer, m_Context->BindlessPipelineLayout, VK_SHADER_STAGE_ALL, offset, size, data);
	}

	void VulkanCommandList::SetViewport(const glm::vec2& size)
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

	void VulkanCommandList::SetScissor(const glm::vec2& size, const glm::vec2& offset)
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

	void VulkanCommandList::SetFillMode(FillMode mode)
	{
		if (m_Context->DynamicState3Supported)
		{
			m_Context->vkCmdSetPolygonModeEXT(CommandBuffer, Utils::GAFillModeToVulkan(mode));
			m_FillModeSet = true;
		}
		else
		{
			m_GraphicsPipelineKey.FillMode = mode;
			m_GraphicsPipelineStateChanged = true;
		}
	}

	void VulkanCommandList::EnableDepthTest(bool writeEnabled, CompareOperation op)
	{
		vkCmdSetDepthBounds(CommandBuffer, 0.0f, 1.0f);
		if (m_Context->DynamicStateSupported)
		{
			m_Context->vkCmdSetDepthTestEnableEXT(CommandBuffer, true);
			m_Context->vkCmdSetDepthWriteEnableEXT(CommandBuffer, writeEnabled);
			m_Context->vkCmdSetDepthCompareOpEXT(CommandBuffer, Utils::GACompareOpToVulkan(op));
			m_DepthEnableSet = true;
		}
		else
		{
			m_GraphicsPipelineKey.DepthTestEnable = true;
			m_GraphicsPipelineKey.DepthWriteEnable = writeEnabled;
			m_GraphicsPipelineKey.DepthCompareOp = op;
			m_GraphicsPipelineStateChanged = true;
		}
	}

	void VulkanCommandList::DisableDepthTest()
	{
		if (m_Context->DynamicRenderingSupported)
		{
			m_Context->vkCmdSetDepthTestEnableEXT(CommandBuffer, false);
			m_DepthEnableSet = true;
		}
		else
		{
			m_GraphicsPipelineKey.DepthTestEnable = false;
			m_GraphicsPipelineStateChanged = true;
		}
	}

	void VulkanCommandList::EnableColorBlend(Blend srcBlend, Blend dstBlend, BlendOp blendOp, Blend srcBlendAlpha, Blend dstBlendAlpha, BlendOp blendAlpha)
	{
		SetColorBlend(true,	srcBlend, dstBlend, blendOp, srcBlendAlpha, dstBlendAlpha, blendAlpha);
	}

	void VulkanCommandList::DisableColorBlend()
	{
		SetColorBlend(false, Blend::Zero, Blend::Zero, BlendOp::Add, Blend::Zero, Blend::Zero, BlendOp::Add);
	}

	void VulkanCommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
	{
		SetDynamicState();
		vkCmdDraw(CommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void VulkanCommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
	{
		SetDynamicState();
		vkCmdDrawIndexed(CommandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void VulkanCommandList::DrawIndirect(const Ref<Buffer>& buffer, uint64_t offset, uint32_t drawCount, uint32_t stride)
	{
		auto& vulkanBuffer = (VulkanBuffer&)(*buffer);

		SetDynamicState();
		vkCmdDrawIndirect(CommandBuffer, vulkanBuffer.Buffer.Buffer, offset + 8, drawCount, stride);
	}

	void VulkanCommandList::DrawIndirectCount(const Ref<Buffer>& buffer, uint64_t offset, const Ref<Buffer>& countBuffer, uint64_t countOffset, uint32_t maxDrawCount, uint32_t stride)
	{
		auto& vulkanBuffer = (VulkanBuffer&)(*buffer);
		auto& vulkanCountBuffer = (VulkanBuffer&)(*countBuffer);

		SetDynamicState();
		vkCmdDrawIndirectCount(CommandBuffer, vulkanBuffer.Buffer.Buffer, offset + 8, vulkanCountBuffer.Buffer.Buffer, countOffset, maxDrawCount, stride);
	}

	void VulkanCommandList::DrawIndexedIndirect(const Ref<Buffer>& buffer, uint64_t offset, uint32_t drawCount, uint32_t stride)
	{
		auto& vulkanBuffer = (VulkanBuffer&)(*buffer);

		SetDynamicState();
		vkCmdDrawIndexedIndirect(CommandBuffer, vulkanBuffer.Buffer.Buffer, offset + 8, drawCount, stride);
	}

	void VulkanCommandList::DrawIndexedIndirectCount(const Ref<Buffer>& buffer, uint64_t offset, const Ref<Buffer>& countBuffer, uint64_t countOffset, uint32_t maxDrawCount, uint32_t stride)
	{
		auto& vulkanBuffer = (VulkanBuffer&)(*buffer);
		auto& vulkanCountBuffer = (VulkanBuffer&)(*countBuffer);

		SetDynamicState();
		vkCmdDrawIndexedIndirectCount(CommandBuffer, vulkanBuffer.Buffer.Buffer, offset + 8, vulkanCountBuffer.Buffer.Buffer, countOffset, maxDrawCount, stride);
	}

	void VulkanCommandList::Dispatch(uint32_t workX, uint32_t workY, uint32_t workZ)
	{
		if (!m_Context->ShaderObjectSupported && m_ComputePipelineStateChanged)
		{
			vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_Context->PipelineManager->GetComputePipeline(m_ComputePipelineKey));
			m_ComputePipelineStateChanged = false;
		}

		vkCmdDispatch(CommandBuffer, workX, workY, workZ);
	}

	void VulkanCommandList::DispatchIndirect(const Ref<Buffer>& buffer, uint64_t offset)
	{
		auto& vulkanBuffer = (VulkanBuffer&)(*buffer);
		if (!m_Context->ShaderObjectSupported && m_ComputePipelineStateChanged)
		{
			vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_Context->PipelineManager->GetComputePipeline(m_ComputePipelineKey));
			m_ComputePipelineStateChanged = false;
		}

		vkCmdDispatchIndirect(CommandBuffer, vulkanBuffer.Buffer.Buffer, offset);
	}

	void VulkanCommandList::SetColorBlend(bool enabled, Blend srcBlend, Blend dstBlend, BlendOp blendOp, Blend srcBlendAlpha, Blend dstBlendAlpha, BlendOp blendAlpha)
	{
		if (m_Context->DynamicState3Supported)
		{
			VkBool32 enable = enabled;
			VkColorBlendEquationEXT equation = {
				.srcColorBlendFactor = Utils::GABlendToVulkan(srcBlend),
				.dstColorBlendFactor = Utils::GABlendToVulkan(dstBlend),
				.colorBlendOp = Utils::GABlendOpToVulkan(blendOp),
				.srcAlphaBlendFactor = Utils::GABlendToVulkan(srcBlendAlpha),
				.dstAlphaBlendFactor = Utils::GABlendToVulkan(dstBlendAlpha),
				.alphaBlendOp = Utils::GABlendOpToVulkan(blendAlpha)
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

	void VulkanCommandList::SetDynamicState()
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
				m_Context->vkCmdSetDepthCompareOpEXT(CommandBuffer, VK_COMPARE_OP_NEVER);
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
			if (!m_FillModeSet) m_Context->vkCmdSetPolygonModeEXT(CommandBuffer, VK_POLYGON_MODE_FILL); 
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