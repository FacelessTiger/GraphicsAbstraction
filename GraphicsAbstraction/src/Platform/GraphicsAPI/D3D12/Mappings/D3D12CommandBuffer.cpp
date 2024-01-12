#include "D3D12CommandBuffer.h"

#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Image.h>
#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Buffer.h>
#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Swapchain.h>
#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Shader.h>
#include <Platform/GraphicsAPI/D3D12/InternalManagers/D3D12Utils.h>
#include <d3dx12/d3dx12.h>

#include <glm/gtc/type_ptr.hpp>

namespace GraphicsAbstraction {

	using namespace Microsoft::WRL;

	struct SpecialConstant
	{
		uint32_t vertexOffset;
		uint32_t instanceOffset;
	};

	D3D12CommandBuffer::D3D12CommandBuffer(ComPtr<ID3D12GraphicsCommandList> commandList)
		: m_Context(D3D12Context::GetReference()), CommandList(commandList)
	{ }

	void D3D12CommandBuffer::Clear(const Ref<Image>&image, const glm::vec4& color)
	{
		auto& d3d12Image = (D3D12Image&)*image;

		d3d12Image.TransitionState(CommandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
		CommandList->ClearRenderTargetView(d3d12Image.CpuHandle, glm::value_ptr(color), 0, nullptr);
	}

	void D3D12CommandBuffer::Present(const Ref<Swapchain>& swapchain)
	{
		auto& d3d12Swapchain = (D3D12Swapchain&)(*swapchain);
		auto image = d3d12Swapchain.Images[d3d12Swapchain.ImageIndex];

		image->TransitionState(CommandList, D3D12_RESOURCE_STATE_PRESENT);
	}

	void D3D12CommandBuffer::CopyToBuffer(const Ref<Buffer>& src, const Ref<Buffer>& dst, uint32_t size, uint32_t srcOffset, uint32_t dstOffset)
	{
		auto& srcBuffer = (D3D12Buffer&)*src;
		auto& dstBuffer = (D3D12Buffer&)*dst;

		dstBuffer.TransitionState(CommandList, D3D12_RESOURCE_STATE_COPY_DEST);

		CommandList->CopyBufferRegion(dstBuffer.Resource.Get(), dstOffset, srcBuffer.Resource.Get(), srcOffset, size);
	}

	void D3D12CommandBuffer::CopyToImage(const Ref<Buffer>& src, const Ref<Image>& dst, uint32_t srcOffset)
	{
		auto& d3d12Buffer = (D3D12Buffer&)*src;
		auto& d3d12Image = (D3D12Image&)*dst;

		d3d12Image.TransitionState(CommandList, D3D12_RESOURCE_STATE_COPY_DEST);
		
		uint32_t bpp = 32; // TODO: make a utility function to calculate this off of format
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT placed = {
			.Offset = srcOffset,
			.Footprint = {
				.Format = Utils::GAImageFormatToD3D12(d3d12Image.Format),
				.Width = d3d12Image.Width,
				.Height = d3d12Image.Height,
				.Depth = 1,
				.RowPitch = (d3d12Image.Width * 32 + 7) / 8
			}
		};

		auto srcCopy = CD3DX12_TEXTURE_COPY_LOCATION(d3d12Buffer.Resource.Get(), placed);
		auto dstCopy = CD3DX12_TEXTURE_COPY_LOCATION(d3d12Image.Image.Get(), 0);	
		CommandList->CopyTextureRegion(&dstCopy, 0, 0, 0, &srcCopy, nullptr);
	}

	void D3D12CommandBuffer::CopyToImage(const Ref<Image>& src, const Ref<Image>& dst)
	{
		auto& d3d12Src = (D3D12Image&)*src;
		auto& d3d12Dst = (D3D12Image&)*dst;

		d3d12Src.TransitionState(CommandList, D3D12_RESOURCE_STATE_COPY_SOURCE);
		d3d12Dst.TransitionState(CommandList, D3D12_RESOURCE_STATE_COPY_DEST);

		auto srcCopy = CD3DX12_TEXTURE_COPY_LOCATION(d3d12Src.Image.Get(), 0);
		auto dstCopy = CD3DX12_TEXTURE_COPY_LOCATION(d3d12Dst.Image.Get(), 0);
		CommandList->CopyTextureRegion(&dstCopy, 0, 0, 0, &srcCopy, nullptr);
	}

	void D3D12CommandBuffer::RWResourceBarrier(const Ref<Image>& resource)
	{
		auto& d3d12Image = (D3D12Image&)*resource;

		auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(d3d12Image.Image.Get());
		CommandList->ResourceBarrier(1, &barrier);
	}

	void D3D12CommandBuffer::BeginRendering(const glm::vec2& region, const std::vector<Ref<Image>>& colorAttachments, const Ref<Image>& depthAttachment)
	{
		auto& d3d12Image = (D3D12Image&)*colorAttachments[0];
		auto& d3d12Depth = (D3D12Image&)*depthAttachment;

		d3d12Image.TransitionState(CommandList, D3D12_RESOURCE_STATE_RENDER_TARGET);

		m_GraphicsPipelineKey.DepthAttachment = ImageFormat::Unknown;
		if (depthAttachment)
		{
			m_GraphicsPipelineKey.DepthAttachment = d3d12Depth.Format;
			CommandList->ClearDepthStencilView(d3d12Depth.CpuHandle, D3D12_CLEAR_FLAG_DEPTH, 0.0f, 0, 0, nullptr);
		}

		m_GraphicsPipelineKey.ColorAttachments[0] = d3d12Image.Format;
		CommandList->OMSetRenderTargets(1, &d3d12Image.CpuHandle, false, depthAttachment ? &d3d12Depth.CpuHandle : nullptr);

		m_GraphicsPipelineStateChanged = true;
	}

	void D3D12CommandBuffer::BindShaders(const std::vector<Ref<Shader>> shaderStages)
	{
		for (auto& shaderStage : shaderStages)
		{
			auto& d3d12Shader = (D3D12Shader&)(*shaderStage);

			switch (d3d12Shader.Stage)
			{
				case ShaderStage::Vertex:
				{
					m_GraphicsPipelineKey.Shaders[0] = d3d12Shader.ID;
					m_GraphicsPipelineStateChanged = true;
					break;
				}
				case ShaderStage::Pixel:
				{
					m_GraphicsPipelineKey.Shaders[4] = d3d12Shader.ID;
					m_GraphicsPipelineStateChanged = true;
					break;
				}
				case ShaderStage::Compute:
				{
					m_ComputePipelineKey.Shader = d3d12Shader.ID;
					m_ComputePipelineStateChanged = true;
					break;
				}
			}
		}
	}

	void D3D12CommandBuffer::BindIndexBuffer(const Ref<Buffer>& buffer)
	{
		auto& d3d12Buffer = (D3D12Buffer&)*buffer;
		d3d12Buffer.TransitionState(CommandList, D3D12_RESOURCE_STATE_INDEX_BUFFER);

		D3D12_INDEX_BUFFER_VIEW view = {
			.BufferLocation = d3d12Buffer.Resource->GetGPUVirtualAddress(),
			.SizeInBytes = d3d12Buffer.Size,
			.Format = DXGI_FORMAT_R16_UINT
		};
		CommandList->IASetIndexBuffer(&view);
	}

	void D3D12CommandBuffer::PushConstant(const void* data, uint32_t size, uint32_t offset)
	{
		CommandList->SetComputeRoot32BitConstants(0, size / 4, data, offset);
		CommandList->SetGraphicsRoot32BitConstants(0, size / 4, data, offset);
	}

	void D3D12CommandBuffer::SetViewport(const glm::vec2& size)
	{
		D3D12_VIEWPORT viewport = {
			.Width = size.x,
			.Height = size.y,
			.MinDepth = 0,
			.MaxDepth = 1
		};
		CommandList->RSSetViewports(1, &viewport);
	}

	void D3D12CommandBuffer::SetScissor(const glm::vec2& size, const glm::vec2& offset)
	{
		D3D12_RECT scissor = {
			.left = (LONG)offset.x,
			.top = (LONG)offset.y,
			.right = (LONG)(size.x + offset.x),
			.bottom = (LONG)(size.y + offset.y),
		};
		CommandList->RSSetScissorRects(1, &scissor);
	}

	void D3D12CommandBuffer::EnableDepthTest(bool writeEnabled, CompareOperation op)
	{
		m_GraphicsPipelineKey.DepthTestEnable = true;
		m_GraphicsPipelineKey.DepthWriteEnable = writeEnabled;
		m_GraphicsPipelineKey.DepthCompareOp = op;
		m_GraphicsPipelineStateChanged = true;
	}

	void D3D12CommandBuffer::DisableDepthTest()
	{
		m_GraphicsPipelineKey.DepthTestEnable = false;
		m_GraphicsPipelineKey.DepthWriteEnable = false;
		m_GraphicsPipelineStateChanged = true;
	}

	void D3D12CommandBuffer::EnableColorBlend(Blend srcBlend, Blend dstBlend, BlendOp blendOp, Blend srcBlendAlpha, Blend dstBlendAlpha, BlendOp blendAlpha)
	{
		m_GraphicsPipelineKey.BlendEnable = true;
		m_GraphicsPipelineKey.SrcBlend = srcBlend;
		m_GraphicsPipelineKey.DstBlend = dstBlend;
		m_GraphicsPipelineKey.BlendOp = blendOp;
		m_GraphicsPipelineKey.SrcBlendAlpha = srcBlendAlpha;
		m_GraphicsPipelineKey.DstBlendAlpha = dstBlendAlpha;
		m_GraphicsPipelineKey.BlendOpAlpha = blendAlpha;
		m_GraphicsPipelineStateChanged = true;
	}

	void D3D12CommandBuffer::DisableColorBlend()
	{
		m_GraphicsPipelineKey.BlendEnable = false;
		m_GraphicsPipelineStateChanged = true;
	}

	void D3D12CommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
	{
		SetGraphicsPipeline();

		SpecialConstant pc = { firstVertex, firstInstance };
		CommandList->SetGraphicsRoot32BitConstants(1, 2, &pc, 0);
		CommandList->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void D3D12CommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
	{
		SetGraphicsPipeline();

		SpecialConstant pc = { vertexOffset, firstInstance };
		CommandList->SetGraphicsRoot32BitConstants(1, 2, &pc, 0);
		CommandList->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void D3D12CommandBuffer::DrawIndirect(const Ref<Buffer>& buffer, uint64_t offset, uint32_t drawCount, uint32_t stride)
	{
		auto& d3d12Buffer = (D3D12Buffer&)*buffer;

		SetGraphicsPipeline();
		d3d12Buffer.TransitionState(CommandList, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
		CommandList->ExecuteIndirect(m_Context->CommandSignatureManager->GetCommandSignature({ D3D12_INDIRECT_ARGUMENT_TYPE_DRAW, stride }), drawCount, d3d12Buffer.Resource.Get(), offset, nullptr, 0);
	}

	void D3D12CommandBuffer::DrawIndirectCount(const Ref<Buffer>& buffer, uint64_t offset, const Ref<Buffer>& countBuffer, uint64_t countOffset, uint32_t maxDrawCount, uint32_t stride)
	{
		auto& d3d12Buffer = (D3D12Buffer&)*buffer;
		auto& d3d12CountBuffer = (D3D12Buffer&)*countBuffer;

		SetGraphicsPipeline();
		d3d12Buffer.TransitionState(CommandList, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
		d3d12CountBuffer.TransitionState(CommandList, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
		CommandList->ExecuteIndirect(m_Context->CommandSignatureManager->GetCommandSignature({ D3D12_INDIRECT_ARGUMENT_TYPE_DRAW, stride }), maxDrawCount, d3d12Buffer.Resource.Get(), offset, d3d12CountBuffer.Resource.Get(), countOffset);
	}

	void D3D12CommandBuffer::DrawIndexedIndirect(const Ref<Buffer>& buffer, uint64_t offset, uint32_t drawCount, uint32_t stride)
	{
		auto& d3d12Buffer = (D3D12Buffer&)*buffer;

		SetGraphicsPipeline();
		d3d12Buffer.TransitionState(CommandList, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
		CommandList->ExecuteIndirect(m_Context->CommandSignatureManager->GetCommandSignature({ D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED, stride }), drawCount, d3d12Buffer.Resource.Get(), offset, nullptr, 0);
	}

	void D3D12CommandBuffer::DrawIndexedIndirectCount(const Ref<Buffer>& buffer, uint64_t offset, const Ref<Buffer>& countBuffer, uint64_t countOffset, uint32_t maxDrawCount, uint32_t stride)
	{
		auto& d3d12Buffer = (D3D12Buffer&)*buffer;
		auto& d3d12CountBuffer = (D3D12Buffer&)*countBuffer;

		SetGraphicsPipeline();
		d3d12Buffer.TransitionState(CommandList, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
		d3d12CountBuffer.TransitionState(CommandList, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
		CommandList->ExecuteIndirect(m_Context->CommandSignatureManager->GetCommandSignature({ D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED, stride }), maxDrawCount, d3d12Buffer.Resource.Get(), offset, d3d12CountBuffer.Resource.Get(), countOffset);
	}

	void D3D12CommandBuffer::Dispatch(uint32_t workX, uint32_t workY, uint32_t workZ)
	{
		if (m_ComputePipelineStateChanged)
		{
			CommandList->SetPipelineState(m_Context->PipelineManager->GetComputePipeline(m_ComputePipelineKey));
			m_ComputePipelineStateChanged = false;
		}

		CommandList->Dispatch(workX, workY, workZ);
	}

	void D3D12CommandBuffer::DispatchIndirect(const Ref<Buffer>& buffer, uint64_t offset)
	{
		auto& d3d12Buffer = (D3D12Buffer&)*buffer;
		if (m_ComputePipelineStateChanged)
		{
			CommandList->SetPipelineState(m_Context->PipelineManager->GetComputePipeline(m_ComputePipelineKey));
			m_ComputePipelineStateChanged = false;
		}

		d3d12Buffer.TransitionState(CommandList, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
		CommandList->ExecuteIndirect(m_Context->CommandSignatureManager->GetCommandSignature({ D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH, sizeof(DispatchIndirectCommand) }), 1, d3d12Buffer.Resource.Get(), offset, nullptr, 0);
	}

	void D3D12CommandBuffer::SetGraphicsPipeline()
	{
		if (m_GraphicsPipelineStateChanged)
		{
			CommandList->SetPipelineState(m_Context->PipelineManager->GetGraphicsPipeline(m_GraphicsPipelineKey));
			m_GraphicsPipelineStateChanged = false;
		}
		if (m_DefaultDynamicStateSet) return;

		CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_DefaultDynamicStateSet = true;
	}

}