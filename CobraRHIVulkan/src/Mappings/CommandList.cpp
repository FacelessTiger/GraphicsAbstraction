#include <VulkanRHI.h>

namespace GraphicsAbstraction {

	CommandList::~CommandList()
	{
		delete impl;
	}

	void CommandList::Clear(const Ref<Image>& image, const glm::vec4& color)
	{
		image->impl->TransitionLayout(impl->CommandBuffer, VK_IMAGE_LAYOUT_GENERAL);

		VkClearColorValue clearValue = { { color.x, color.y, color.z, color.w } };
		VkImageSubresourceRange clearRange = Utils::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

		vkCmdClearColorImage(impl->CommandBuffer, image->impl->Image.Image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
	}

	void CommandList::Present(const Ref<Swapchain>& swapchain)
	{
		auto image = swapchain->impl->Images[swapchain->impl->ImageIndex];
		image->impl->TransitionLayout(impl->CommandBuffer, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}

	void CommandList::CopyBufferRegion(const Ref<Buffer>& src, const Ref<Buffer>& dst, uint32_t size, uint32_t srcOffset, uint32_t dstOffset)
	{
		VkBufferCopy copy = {
			.srcOffset = srcOffset,
			.dstOffset = dstOffset,
			.size = size
		};
		vkCmdCopyBuffer(impl->CommandBuffer, src->impl->Buffer.Buffer, dst->impl->Buffer.Buffer, 1, &copy);
	}

	void CommandList::CopyToImage(const Ref<Buffer>& src, const Ref<Image>& dst, uint32_t srcOffset)
	{
		dst->impl->TransitionLayout(impl->CommandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkBufferImageCopy copy = {
			.bufferOffset = srcOffset,
			.bufferRowLength = 0,
			.bufferImageHeight = 0
		};

		copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy.imageSubresource.mipLevel = 0;
		copy.imageSubresource.baseArrayLayer = 0;
		copy.imageSubresource.layerCount = 1;
		copy.imageExtent = { dst->impl->Width, dst->impl->Height, 1 };

		vkCmdCopyBufferToImage(impl->CommandBuffer, src->impl->Buffer.Buffer, dst->impl->Image.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
		dst->impl->TransitionLayout(impl->CommandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void CommandList::CopyToImage(const Ref<Image>& src, const Ref<Image>& dst)
	{
		src->impl->TransitionLayout(impl->CommandBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		dst->impl->TransitionLayout(impl->CommandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkImageCopy copy = {
			.extent = { src->impl->Width, src->impl->Height, 1 }
		};
		copy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy.srcSubresource.layerCount = 1;

		copy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy.dstSubresource.layerCount = 1;

		vkCmdCopyImage(impl->CommandBuffer, src->impl->Image.Image, src->impl->Layout, dst->impl->Image.Image, dst->impl->Layout, 1, &copy);
	}

	void CommandList::RWResourceBarrier(const Ref<Image>& resource)
	{
		if (resource->impl->Layout == VK_IMAGE_LAYOUT_UNDEFINED) return;
		
		VkImageMemoryBarrier barrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
			.oldLayout = resource->impl->Layout,
			.newLayout = resource->impl->Layout,

			.image = resource->impl->Image.Image,
			.subresourceRange = Utils::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT)
		};
		vkCmdPipelineBarrier(impl->CommandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	void CommandList::RWResourceBarrier(const Ref<Buffer>& resource)
	{
		VkBufferMemoryBarrier barrier = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
			.buffer = resource->impl->Buffer.Buffer,
			.size = VK_WHOLE_SIZE
		};
		vkCmdPipelineBarrier(impl->CommandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 1, &barrier, 0, nullptr);
	}

	void CommandList::BeginRendering(const glm::vec2& region, const std::vector<Ref<Image>>& colorAttachments, const Ref<Image>& depthAttachment)
	{
		if (g_DynamicRenderingSupported)
		{
			std::vector<VkRenderingAttachmentInfo> vulkanColorAttachments;
			VkRenderingAttachmentInfo vulkanDepthAttachment = {};

			for (int i = 0; i < colorAttachments.size(); i++)
			{
				auto imageImpl = colorAttachments[i]->impl;
				imageImpl->TransitionLayout(impl->CommandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
				impl->GraphicsPipelineKey.ColorAttachments[i] = imageImpl->Format;

				vulkanColorAttachments.push_back({
					.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
					.imageView = imageImpl->View,
					.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE
				});
			}

			impl->GraphicsPipelineKey.DepthAttachment = ImageFormat::Unknown;
			if (depthAttachment)
			{
				auto imageImpl = depthAttachment->impl;
				imageImpl->TransitionLayout(impl->CommandBuffer, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
				impl->GraphicsPipelineKey.DepthAttachment = imageImpl->Format;

				vulkanDepthAttachment = {
					.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
					.imageView = imageImpl->View,
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

			impl->GraphicsPipelineStateChanged = true;
			impl->Context->vkCmdBeginRenderingKHR(impl->CommandBuffer, &renderInfo);
		}
		else
		{
			RenderInfoKey key;
			key.Width = (uint32_t)region.x;
			key.Height = (uint32_t)region.y;

			std::vector<VkImageView> attachmentViews;
			attachmentViews.reserve(colorAttachments.size());

			if (depthAttachment)
			{
				auto imageImpl = depthAttachment->impl;
				key.DepthAttachment = { imageImpl->Format, imageImpl->Layout, imageImpl->Usage };

				imageImpl->Layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				attachmentViews.push_back(imageImpl->View);
			}

			for (int i = 0; i < colorAttachments.size(); i++)
			{
				auto& imageImpl = colorAttachments[i]->impl;
				key.ColorAttachments[i] = { imageImpl->Format, imageImpl->Layout, imageImpl->Usage };

				imageImpl->Layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				attachmentViews.push_back(imageImpl->View);
			}

			RenderInfo info = impl->Context->RenderInfoManager->GetRenderInfo(key);
			impl->GraphicsPipelineKey.Renderpass = info.renderpass;

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

			impl->GraphicsPipelineStateChanged = true;
			vkCmdBeginRenderPass(impl->CommandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
		}
	}

	void CommandList::EndRendering()
	{
		if (g_DynamicRenderingSupported) impl->Context->vkCmdEndRenderingKHR(impl->CommandBuffer);
		else vkCmdEndRenderPass(impl->CommandBuffer);
	}

	void CommandList::BindShaders(const std::vector<Ref<Shader>> shaderStages)
	{
		std::vector<VkShaderStageFlagBits> stages;
		std::vector<VkShaderEXT> shaders;
		stages.reserve(shaderStages.size());
		shaders.reserve(shaderStages.size());

		for (auto& shaderStage : shaderStages)
		{
			auto shaderImpl = shaderStage->impl;
			if (g_ShaderObjectSupported)
			{
				stages.push_back(shaderImpl->Stage);
				shaders.push_back(shaderImpl->ShaderObject);
			}
			else
			{
				switch (shaderImpl->Stage)
				{
					case VK_SHADER_STAGE_VERTEX_BIT:
					{
						impl->GraphicsPipelineKey.Shaders[0] = shaderImpl->ID;
						impl->GraphicsPipelineStateChanged = true;
						break;
					}
					case VK_SHADER_STAGE_FRAGMENT_BIT:
					{
						impl->GraphicsPipelineKey.Shaders[4] = shaderImpl->ID;
						impl->GraphicsPipelineStateChanged = true;
						break;
					}
					case VK_SHADER_STAGE_COMPUTE_BIT:
					{
						impl->ComputePipelineKey.Shader = shaderImpl->ID;
						impl->ComputePipelineStateChanged = true;
						break;
					}
				}
			}
		}

		if (g_ShaderObjectSupported) impl->Context->vkCmdBindShadersEXT(impl->CommandBuffer, (uint32_t)shaders.size(), stages.data(), shaders.data());
	}

	void CommandList::BindIndexBuffer(const Ref<Buffer>& buffer, IndexType type)
	{
		vkCmdBindIndexBuffer(impl->CommandBuffer, buffer->impl->Buffer.Buffer, 0, (type == IndexType::Uint16) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
	}

	void CommandList::PushConstant(const void* data, uint32_t size, uint32_t offset)
	{
		vkCmdPushConstants(impl->CommandBuffer, impl->Context->BindlessPipelineLayout, VK_SHADER_STAGE_ALL, offset, size, data);
	}

	void CommandList::SetViewport(const glm::vec2& size)
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

		if (g_DynamicStateSupported) impl->Context->vkCmdSetViewportWithCountEXT(impl->CommandBuffer, 1, &viewport);
		else vkCmdSetViewport(impl->CommandBuffer, 0, 1, &viewport);
	}

	void CommandList::SetScissor(const glm::vec2& size, const glm::vec2& offset)
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

		if (g_DynamicStateSupported) impl->Context->vkCmdSetScissorWithCountEXT(impl->CommandBuffer, 1, &scissor);
		else vkCmdSetScissor(impl->CommandBuffer, 0, 1, &scissor);
	}

	void CommandList::SetFillMode(FillMode mode)
	{
		if (g_DynamicState3Supported)
		{
			impl->Context->vkCmdSetPolygonModeEXT(impl->CommandBuffer, Utils::GAFillModeToVulkan(mode));
			impl->FillModeSet = true;
		}
		else
		{
			impl->GraphicsPipelineKey.FillMode = mode;
			impl->GraphicsPipelineStateChanged = true;
		}
	}

	void CommandList::EnableDepthTest(bool writeEnabled, CompareOperation op)
	{
		vkCmdSetDepthBounds(impl->CommandBuffer, 0.0f, 1.0f);
		if (g_DynamicStateSupported)
		{
			impl->Context->vkCmdSetDepthTestEnableEXT(impl->CommandBuffer, true);
			impl->Context->vkCmdSetDepthWriteEnableEXT(impl->CommandBuffer, writeEnabled);
			impl->Context->vkCmdSetDepthCompareOpEXT(impl->CommandBuffer, Utils::GACompareOpToVulkan(op));
			impl->DepthEnableSet = true;
		}
		else
		{
			impl->GraphicsPipelineKey.DepthTestEnable = true;
			impl->GraphicsPipelineKey.DepthWriteEnable = writeEnabled;
			impl->GraphicsPipelineKey.DepthCompareOp = op;
			impl->GraphicsPipelineStateChanged = true;
		}
	}

	void CommandList::DisableDepthTest()
	{
		if (g_DynamicRenderingSupported)
		{
			impl->Context->vkCmdSetDepthTestEnableEXT(impl->CommandBuffer, false);
			impl->DepthEnableSet = true;
		}
		else
		{
			impl->GraphicsPipelineKey.DepthTestEnable = false;
			impl->GraphicsPipelineStateChanged = true;
		}
	}

	void CommandList::EnableColorBlend(Blend srcBlend, Blend dstBlend, BlendOp blendOp, Blend srcBlendAlpha, Blend dstBlendAlpha, BlendOp blendAlpha)
	{
		impl->SetColorBlend(true, srcBlend, dstBlend, blendOp, srcBlendAlpha, dstBlendAlpha, blendAlpha);
	}

	void CommandList::DisableColorBlend()
	{
		impl->SetColorBlend(false, Blend::Zero, Blend::Zero, BlendOp::Add, Blend::Zero, Blend::Zero, BlendOp::Add);
	}

	void CommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
	{
		impl->SetDynamicState();
		vkCmdDraw(impl->CommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void CommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
	{
		impl->SetDynamicState();
		vkCmdDrawIndexed(impl->CommandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void CommandList::DrawIndirect(const Ref<Buffer>& buffer, uint64_t offset, uint32_t drawCount, uint32_t stride)
	{
		impl->SetDynamicState();
		vkCmdDrawIndirect(impl->CommandBuffer, buffer->impl->Buffer.Buffer, offset + 8, drawCount, stride);
	}

	void CommandList::DrawIndirectCount(const Ref<Buffer>& buffer, uint64_t offset, const Ref<Buffer>& countBuffer, uint64_t countOffset, uint32_t maxDrawCount, uint32_t stride)
	{
		impl->SetDynamicState();
		vkCmdDrawIndirectCount(impl->CommandBuffer, buffer->impl->Buffer.Buffer, offset + 8, countBuffer->impl->Buffer.Buffer, countOffset, maxDrawCount, stride);
	}

	void CommandList::DrawIndexedIndirect(const Ref<Buffer>& buffer, uint64_t offset, uint32_t drawCount, uint32_t stride)
	{
		impl->SetDynamicState();
		vkCmdDrawIndexedIndirect(impl->CommandBuffer, buffer->impl->Buffer.Buffer, offset + 8, drawCount, stride);
	}

	void CommandList::DrawIndexedIndirectCount(const Ref<Buffer>& buffer, uint64_t offset, const Ref<Buffer>& countBuffer, uint64_t countOffset, uint32_t maxDrawCount, uint32_t stride)
	{
		impl->SetDynamicState();
		vkCmdDrawIndexedIndirectCount(impl->CommandBuffer, buffer->impl->Buffer.Buffer, offset + 8, countBuffer->impl->Buffer.Buffer, countOffset, maxDrawCount, stride);
	}

	void CommandList::Dispatch(uint32_t workX, uint32_t workY, uint32_t workZ)
	{
		if (!g_ShaderObjectSupported && impl->ComputePipelineStateChanged)
		{
			vkCmdBindPipeline(impl->CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, impl->Context->PipelineManager->GetComputePipeline(impl->ComputePipelineKey));
			impl->ComputePipelineStateChanged = false;
		}

		vkCmdDispatch(impl->CommandBuffer, workX, workY, workZ);
	}

	void CommandList::DispatchIndirect(const Ref<Buffer>& buffer, uint64_t offset)
	{
		if (!g_ShaderObjectSupported && impl->ComputePipelineStateChanged)
		{
			vkCmdBindPipeline(impl->CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, impl->Context->PipelineManager->GetComputePipeline(impl->ComputePipelineKey));
			impl->ComputePipelineStateChanged = false;
		}

		vkCmdDispatchIndirect(impl->CommandBuffer, buffer->impl->Buffer.Buffer, offset);
	}

	Impl<CommandList>::Impl(VkCommandBuffer buffer)
		: Context(Impl<GraphicsContext>::Reference), CommandBuffer(buffer)
	{ }

	void Impl<CommandList>::SetColorBlend(bool enabled, Blend srcBlend, Blend dstBlend, BlendOp blendOp, Blend srcBlendAlpha, Blend dstBlendAlpha, BlendOp blendAlpha)
	{
		if (g_DynamicState3Supported)
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

			Context->vkCmdSetColorBlendEnableEXT(CommandBuffer, 0, 1, &enable);
			Context->vkCmdSetColorBlendEquationEXT(CommandBuffer, 0, 1, &equation);
			ColorBlendSet = true;
		}
		else
		{
			GraphicsPipelineKey.BlendEnable = enabled;
			GraphicsPipelineKey.SrcBlend = srcBlend;
			GraphicsPipelineKey.DstBlend = dstBlend;
			GraphicsPipelineKey.BlendOp = blendOp;
			GraphicsPipelineKey.SrcBlendAlpha = srcBlendAlpha;
			GraphicsPipelineKey.DstBlendAlpha = dstBlendAlpha;
			GraphicsPipelineKey.BlendOpAlpha = blendAlpha;
			GraphicsPipelineStateChanged = true;
		}
	}

	void Impl<CommandList>::SetDynamicState()
	{
		if (!g_ShaderObjectSupported && GraphicsPipelineStateChanged)
		{
			vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Context->PipelineManager->GetGraphicsPipeline(GraphicsPipelineKey));
			GraphicsPipelineStateChanged = false;
		}
		if (DefaultDynamicStateSet) return;

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

		if (g_DynamicStateSupported)
		{
			Context->vkCmdSetCullModeEXT(CommandBuffer, VK_CULL_MODE_NONE);
			Context->vkCmdSetFrontFaceEXT(CommandBuffer, VK_FRONT_FACE_CLOCKWISE);
			Context->vkCmdSetPrimitiveTopologyEXT(CommandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

			if (!DepthEnableSet)
			{
				Context->vkCmdSetDepthTestEnableEXT(CommandBuffer, false);
				Context->vkCmdSetDepthWriteEnableEXT(CommandBuffer, false);
				Context->vkCmdSetDepthCompareOpEXT(CommandBuffer, VK_COMPARE_OP_NEVER);
			}

			Context->vkCmdSetDepthBoundsTestEnableEXT(CommandBuffer, false);
			Context->vkCmdSetStencilTestEnableEXT(CommandBuffer, false);
			Context->vkCmdSetStencilOpEXT(CommandBuffer, VK_STENCIL_FACE_FRONT_BIT, {}, {}, {}, {});
		}

		if (g_DynamicState2Supported)
		{
			Context->vkCmdSetRasterizerDiscardEnableEXT(CommandBuffer, false);
			Context->vkCmdSetDepthBiasEnableEXT(CommandBuffer, false);
			Context->vkCmdSetPrimitiveRestartEnableEXT(CommandBuffer, false);
		}

		if (g_DynamicState3Supported)
		{
			if (!FillModeSet) Context->vkCmdSetPolygonModeEXT(CommandBuffer, VK_POLYGON_MODE_FILL); 
			Context->vkCmdSetRasterizationSamplesEXT(CommandBuffer, VK_SAMPLE_COUNT_1_BIT);
			Context->vkCmdSetSampleMaskEXT(CommandBuffer, VK_SAMPLE_COUNT_1_BIT, &mask);
			Context->vkCmdSetAlphaToCoverageEnableEXT(CommandBuffer, false);

			if (!ColorBlendSet)
			{
				Context->vkCmdSetColorBlendEnableEXT(CommandBuffer, 0, 1, &colorBlendEnable);
				Context->vkCmdSetColorBlendEquationEXT(CommandBuffer, 0, 1, &colorBlendEquation);
			}

			Context->vkCmdSetColorWriteMaskEXT(CommandBuffer, 0, 1, &colorWriteMask);
		}

		if (g_ShaderObjectSupported) Context->vkCmdSetVertexInputEXT(CommandBuffer, 0, nullptr, 0, nullptr);

		DefaultDynamicStateSet = true;
	}

}