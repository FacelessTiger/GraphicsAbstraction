#include <DirectXRHI.h>

#include <glm/gtc/type_ptr.hpp>

namespace GraphicsAbstraction {

	struct SpecialConstant
	{
		uint32_t vertexOffset;
		uint32_t instanceOffset;
	};

	CommandList::~CommandList()
	{
		delete impl;
	}

	void CommandList::Clear(const Ref<Image>&image, const glm::vec4& color)
	{
		image->impl->TransitionState(impl->CommandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
		impl->CommandList->ClearRenderTargetView(image->impl->CpuHandle, glm::value_ptr(color), 0, nullptr);
	}

	void CommandList::Clear(const Ref<Image>& image, const glm::ivec4& color)
	{
		float floatValues[4] = { (float)color.r, (float)color.g, (float)color.b, (float)color.a };

		image->impl->TransitionState(impl->CommandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
		impl->CommandList->ClearRenderTargetView(image->impl->CpuHandle, floatValues, 0, nullptr);
	}

	void CommandList::Present(const Ref<Swapchain>& swapchain)
	{
		auto image = swapchain->impl->Images[swapchain->impl->ImageIndex];
		image->impl->TransitionState(impl->CommandList, D3D12_RESOURCE_STATE_PRESENT);
	}

	void CommandList::CopyBufferRegion(const Ref<Buffer>& src, const Ref<Buffer>& dst, uint32_t size, uint32_t srcOffset, uint32_t dstOffset)
	{
		dst->impl->TransitionState(impl->CommandList, D3D12_RESOURCE_STATE_COPY_DEST);
		impl->CommandList->CopyBufferRegion(dst->impl->AResource.Resource.Get(), dstOffset, src->impl->AResource.Resource.Get(), srcOffset, size);
	}

	void CommandList::CopyImageToBuffer(const Ref<Image>& src, const Ref<Buffer>& dst)
	{
		src->impl->TransitionState(impl->CommandList, D3D12_RESOURCE_STATE_COPY_SOURCE);
		dst->impl->TransitionState(impl->CommandList, D3D12_RESOURCE_STATE_COPY_DEST);

		uint32_t bpp = 32; // TODO: make a utility function to calculate this off of format
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT placed = {
			.Offset = 0,
			.Footprint = {
				.Format = Utils::GAImageFormatToD3D12(src->impl->Format),
				.Width = src->impl->Width,
				.Height = src->impl->Height,
				.Depth = 1,
				.RowPitch = (src->impl->Width * bpp + 7) / 8
			}
		};

		auto srcCopy = CD3DX12_TEXTURE_COPY_LOCATION(src->impl->AResource.Resource.Get());
		auto dstCopy = CD3DX12_TEXTURE_COPY_LOCATION(dst->impl->AResource.Resource.Get(), placed);
		impl->CommandList->CopyTextureRegion(&dstCopy, 0, 0, 0, &srcCopy, nullptr);
	}

	void CommandList::CopyToImage(const Ref<Buffer>& src, const Ref<Image>& dst, uint32_t srcOffset)
	{
		dst->impl->TransitionState(impl->CommandList, D3D12_RESOURCE_STATE_COPY_DEST);
		
		uint32_t bpp = 32; // TODO: make a utility function to calculate this off of format
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT placed = {
			.Offset = srcOffset,
			.Footprint = {
				.Format = Utils::GAImageFormatToD3D12(dst->impl->Format),
				.Width = dst->impl->Width,
				.Height = dst->impl->Height,
				.Depth = 1,
				.RowPitch = (dst->impl->Width * bpp + 7) / 8
			}
		};

		auto srcCopy = CD3DX12_TEXTURE_COPY_LOCATION(src->impl->AResource.Resource.Get(), placed);
		auto dstCopy = CD3DX12_TEXTURE_COPY_LOCATION(dst->impl->AResource.Resource.Get(), 0);
		impl->CommandList->CopyTextureRegion(&dstCopy, 0, 0, 0, &srcCopy, nullptr);
	}

	void CommandList::CopyToImage(const Ref<Image>& src, const Ref<Image>& dst)
	{
		src->impl->TransitionState(impl->CommandList, D3D12_RESOURCE_STATE_COPY_SOURCE);
		dst->impl->TransitionState(impl->CommandList, D3D12_RESOURCE_STATE_COPY_DEST);

		auto srcCopy = CD3DX12_TEXTURE_COPY_LOCATION(src->impl->AResource.Resource.Get(), 0);
		auto dstCopy = CD3DX12_TEXTURE_COPY_LOCATION(dst->impl->AResource.Resource.Get(), 0);
		impl->CommandList->CopyTextureRegion(&dstCopy, 0, 0, 0, &srcCopy, nullptr);
	}

	void CommandList::RWResourceBarrier(const Ref<Image>& resource)
	{
		auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(resource->impl->AResource.Resource.Get());
		impl->CommandList->ResourceBarrier(1, &barrier);
	}

	void CommandList::RWResourceBarrier(const Ref<Buffer>& resource)
	{
		auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(resource->impl->AResource.Resource.Get());
		impl->CommandList->ResourceBarrier(1, &barrier);
	}

	void CommandList::BeginRendering(const glm::vec2& region, const std::vector<Ref<Image>>& colorAttachments, const Ref<Image>& depthAttachment)
	{
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> colorAttachmentHandles;
		for (int i = 0; i < impl->GraphicsPipelineKey.ColorAttachments.size(); i++)
		{
			if (i < colorAttachments.size())
			{
				auto imageImpl = colorAttachments[i]->impl;
				imageImpl->TransitionState(impl->CommandList, D3D12_RESOURCE_STATE_RENDER_TARGET);

				impl->GraphicsPipelineKey.ColorAttachments[i].Format = imageImpl->Format;
				colorAttachmentHandles.push_back(imageImpl->CpuHandle);
			}
			else
			{
				impl->GraphicsPipelineKey.ColorAttachments[i].Format = ImageFormat::Unknown;
			}
		}

		impl->GraphicsPipelineKey.DepthAttachment = ImageFormat::Unknown;
		if (depthAttachment)
		{
			impl->GraphicsPipelineKey.DepthAttachment = depthAttachment->impl->Format;
			impl->CommandList->ClearDepthStencilView(depthAttachment->impl->CpuHandle, D3D12_CLEAR_FLAG_DEPTH, 0.0f, 0, 0, nullptr);
		}

		impl->CommandList->OMSetRenderTargets(colorAttachments.size(), colorAttachmentHandles.data(), false, depthAttachment ? &depthAttachment->impl->CpuHandle : nullptr);
		impl->GraphicsPipelineStateChanged = true;
	}

	void CommandList::EndRendering() { }

	void CommandList::BindShaders(const std::vector<Ref<Shader>> shaderStages)
	{
		for (auto& shaderStage : shaderStages)
		{
			switch (shaderStage->impl->Stage)
			{
				case ShaderStage::Vertex:
				{
					impl->GraphicsPipelineKey.Shaders[0] = shaderStage->impl->ID;
					impl->GraphicsPipelineStateChanged = true;
					break;
				}
				case ShaderStage::Pixel:
				{
					impl->GraphicsPipelineKey.Shaders[4] = shaderStage->impl->ID;
					impl->GraphicsPipelineStateChanged = true;
					break;
				}
				case ShaderStage::Compute:
				{
					impl->ComputePipelineKey.Shader = shaderStage->impl->ID;
					impl->ComputePipelineStateChanged = true;
					break;
				}
			}
		}
	}

	void CommandList::BindIndexBuffer(const Ref<Buffer>& buffer, IndexType type)
	{
		buffer->impl->TransitionState(impl->CommandList, D3D12_RESOURCE_STATE_INDEX_BUFFER);
		D3D12_INDEX_BUFFER_VIEW view = {
			.BufferLocation = buffer->impl->AResource.Resource->GetGPUVirtualAddress(),
			.SizeInBytes = buffer->impl->Size,
			.Format = (type == IndexType::Uint16) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT
		};
		impl->CommandList->IASetIndexBuffer(&view);
	}

	void CommandList::PushConstant(const void* data, uint32_t size, uint32_t offset)
	{
		impl->CommandList->SetComputeRoot32BitConstants(0, size / 4, data, offset);
		impl->CommandList->SetGraphicsRoot32BitConstants(0, size / 4, data, offset);
	}

	void CommandList::SetViewport(const glm::vec2& size)
	{
		D3D12_VIEWPORT viewport = {
			.Width = size.x,
			.Height = size.y,
			.MinDepth = 0,
			.MaxDepth = 1
		};
		impl->CommandList->RSSetViewports(1, &viewport);
	}

	void CommandList::SetScissor(const glm::vec2& size, const glm::vec2& offset)
	{
		D3D12_RECT scissor = {
			.left = (LONG)offset.x,
			.top = (LONG)offset.y,
			.right = (LONG)(size.x + offset.x),
			.bottom = (LONG)(size.y + offset.y),
		};
		impl->CommandList->RSSetScissorRects(1, &scissor);
	}

	void CommandList::SetFillMode(FillMode mode)
	{
		impl->GraphicsPipelineKey.FillMode = mode;
		impl->GraphicsPipelineStateChanged = true;
	}

	void CommandList::EnableDepthTest(bool writeEnabled, CompareOperation op)
	{
		impl->GraphicsPipelineKey.DepthTestEnable = true;
		impl->GraphicsPipelineKey.DepthWriteEnable = writeEnabled;
		impl->GraphicsPipelineKey.DepthCompareOp = op;
		impl->GraphicsPipelineStateChanged = true;
	}

	void CommandList::DisableDepthTest()
	{
		impl->GraphicsPipelineKey.DepthTestEnable = false;
		impl->GraphicsPipelineKey.DepthWriteEnable = false;
		impl->GraphicsPipelineStateChanged = true;
	}

	void CommandList::EnableColorBlend(Blend srcBlend, Blend dstBlend, BlendOp blendOp, Blend srcBlendAlpha, Blend dstBlendAlpha, BlendOp blendAlpha, uint32_t attachment)
	{
		BlendInfo& info = impl->GraphicsPipelineKey.ColorAttachments[attachment].BlendInfo;
		info.BlendEnable = true;
		info.SrcBlend = srcBlend;
		info.DstBlend = dstBlend;
		info.BlendOp = blendOp;
		info.SrcBlendAlpha = srcBlendAlpha;
		info.DstBlendAlpha = dstBlendAlpha;
		info.BlendOpAlpha = blendAlpha;
		impl->GraphicsPipelineStateChanged = true;
	}

	void CommandList::DisableColorBlend(uint32_t attachment)
	{
		BlendInfo& info = impl->GraphicsPipelineKey.ColorAttachments[attachment].BlendInfo;
		info.BlendEnable = false;
		impl->GraphicsPipelineStateChanged = true;
	}

	void CommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
	{
		impl->SetGraphicsPipeline();

		SpecialConstant pc = { firstVertex, firstInstance };
		impl->CommandList->SetGraphicsRoot32BitConstants(1, 2, &pc, 0);
		impl->CommandList->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void CommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
	{
		impl->SetGraphicsPipeline();

		SpecialConstant pc = { vertexOffset, firstInstance };
		impl->CommandList->SetGraphicsRoot32BitConstants(1, 2, &pc, 0);
		impl->CommandList->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void CommandList::DrawIndirect(const Ref<Buffer>& buffer, uint64_t offset, uint32_t drawCount, uint32_t stride)
	{
		impl->SetGraphicsPipeline();
		buffer->impl->TransitionState(impl->CommandList, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
		impl->CommandList->ExecuteIndirect(impl->Context->CommandSignatureManager->GetCommandSignature({ D3D12_INDIRECT_ARGUMENT_TYPE_DRAW, stride }), drawCount, buffer->impl->AResource.Resource.Get(), offset, nullptr, 0);
	}

	void CommandList::DrawIndirectCount(const Ref<Buffer>& buffer, uint64_t offset, const Ref<Buffer>& countBuffer, uint64_t countOffset, uint32_t maxDrawCount, uint32_t stride)
	{
		impl->SetGraphicsPipeline();
		buffer->impl->TransitionState(impl->CommandList, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
		countBuffer->impl->TransitionState(impl->CommandList, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
		impl->CommandList->ExecuteIndirect(impl->Context->CommandSignatureManager->GetCommandSignature({ D3D12_INDIRECT_ARGUMENT_TYPE_DRAW, stride }), maxDrawCount, buffer->impl->AResource.Resource.Get(), offset, countBuffer->impl->AResource.Resource.Get(), countOffset);
	}

	void CommandList::DrawIndexedIndirect(const Ref<Buffer>& buffer, uint64_t offset, uint32_t drawCount, uint32_t stride)
	{
		impl->SetGraphicsPipeline();
		buffer->impl->TransitionState(impl->CommandList, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
		impl->CommandList->ExecuteIndirect(impl->Context->CommandSignatureManager->GetCommandSignature({ D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED, stride }), drawCount, buffer->impl->AResource.Resource.Get(), offset, nullptr, 0);
	}

	void CommandList::DrawIndexedIndirectCount(const Ref<Buffer>& buffer, uint64_t offset, const Ref<Buffer>& countBuffer, uint64_t countOffset, uint32_t maxDrawCount, uint32_t stride)
	{
		impl->SetGraphicsPipeline();
		buffer->impl->TransitionState(impl->CommandList, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
		countBuffer->impl->TransitionState(impl->CommandList, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
		impl->CommandList->ExecuteIndirect(impl->Context->CommandSignatureManager->GetCommandSignature({ D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED, stride }), maxDrawCount, buffer->impl->AResource.Resource.Get(), offset, countBuffer->impl->AResource.Resource.Get(), countOffset);
	}

	void CommandList::Dispatch(uint32_t workX, uint32_t workY, uint32_t workZ)
	{
		if (impl->ComputePipelineStateChanged)
		{
			impl->CommandList->SetPipelineState(impl->Context->PipelineManager->GetComputePipeline(impl->ComputePipelineKey));
			impl->ComputePipelineStateChanged = false;
		}

		impl->CommandList->Dispatch(workX, workY, workZ);
	}

	void CommandList::DispatchIndirect(const Ref<Buffer>& buffer, uint64_t offset)
	{
		if (impl->ComputePipelineStateChanged)
		{
			impl->CommandList->SetPipelineState(impl->Context->PipelineManager->GetComputePipeline(impl->ComputePipelineKey));
			impl->ComputePipelineStateChanged = false;
		}

		buffer->impl->TransitionState(impl->CommandList, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
		impl->CommandList->ExecuteIndirect(impl->Context->CommandSignatureManager->GetCommandSignature({ D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH, sizeof(DispatchIndirectCommand) }), 1, buffer->impl->AResource.Resource.Get(), offset, nullptr, 0);
	}

	Impl<CommandList>::Impl(ComPtr<ID3D12GraphicsCommandList> commandList)
		: Context(Impl<GraphicsContext>::Reference), CommandList(commandList)
	{ }

	void Impl<CommandList>::SetGraphicsPipeline()
	{
		if (GraphicsPipelineStateChanged)
		{
			CommandList->SetPipelineState(Context->PipelineManager->GetGraphicsPipeline(GraphicsPipelineKey));
			GraphicsPipelineStateChanged = false;
		}
		if (DefaultDynamicStateSet) return;

		CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		DefaultDynamicStateSet = true;
	}

}