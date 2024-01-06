#include "D3D12CommandBuffer.h"

#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Image.h>
#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Buffer.h>
#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Swapchain.h>
#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Shader.h>
#include <d3dx12/d3dx12.h>

#include <glm/gtc/type_ptr.hpp>

namespace GraphicsAbstraction {

	using namespace Microsoft::WRL;

	D3D12CommandBuffer::D3D12CommandBuffer(ComPtr<ID3D12GraphicsCommandList> commandList)
		: m_Context(D3D12Context::GetReference()), CommandList(commandList)
	{ }

	void D3D12CommandBuffer::Clear(const Ref<Image>&image, const glm::vec4& color)
	{
		auto& vulkanImage = (D3D12Image&)*image;

		vulkanImage.TransitionState(CommandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
		CommandList->ClearRenderTargetView(vulkanImage.CpuHandle, glm::value_ptr(color), 0, nullptr);
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

		CommandList->CopyBufferRegion(dstBuffer.Resource.Get(), dstOffset, srcBuffer.Resource.Get(), srcOffset, size);
	}

	void D3D12CommandBuffer::BeginRendering(const glm::vec2& region, const std::vector<Ref<Image>>& colorAttachments, const Ref<Image>& depthAttachment)
	{
		auto& d3d12Image = (D3D12Image&)*colorAttachments[0];
		auto& d3d12Depth = (D3D12Image&)*depthAttachment;

		m_GraphicsPipelineKey.ColorAttachments[0] = d3d12Image.Format;
		m_GraphicsPipelineStateChanged = true;

		CommandList->OMSetRenderTargets(1, &d3d12Image.CpuHandle, false, &d3d12Depth.CpuHandle);
		CommandList->ClearDepthStencilView(d3d12Depth.CpuHandle, D3D12_CLEAR_FLAG_DEPTH, 0.0f, 0, 0, nullptr);
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
			}
		}
	}

	void D3D12CommandBuffer::BindIndexBuffer(const Ref<Buffer>& buffer)
	{
		auto& d3d12Buffer = (D3D12Buffer&)*buffer;

		D3D12_INDEX_BUFFER_VIEW view = {
			.BufferLocation = d3d12Buffer.Resource->GetGPUVirtualAddress(),
			.SizeInBytes = d3d12Buffer.Size,
			.Format = DXGI_FORMAT_R16_UINT
		};
		CommandList->IASetIndexBuffer(&view);
	}

	void D3D12CommandBuffer::PushConstant(const void* data, uint32_t size, uint32_t offset)
	{
		CommandList->SetGraphicsRoot32BitConstants(0, size / 4, data, 0);
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
			.left = 0,
			.top = 0,
			.right = (LONG)size.x,
			.bottom = (LONG)size.y,
		};
		CommandList->RSSetScissorRects(1, &scissor);
	}

	void D3D12CommandBuffer::SetDepthTest(bool testEnabled, bool writeEnabled, CompareOperation op)
	{
		m_GraphicsPipelineKey.DepthTestEnable = testEnabled;
		m_GraphicsPipelineKey.DepthWriteEnable = writeEnabled;
		m_GraphicsPipelineKey.DepthCompareOp = op;
		m_GraphicsPipelineStateChanged = true;
	}

	void D3D12CommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
	{
		SetGraphicsPipeline();
		CommandList->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void D3D12CommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
	{
		SetGraphicsPipeline();
		CommandList->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void D3D12CommandBuffer::SetGraphicsPipeline()
	{
		if (m_GraphicsPipelineStateChanged)
		{
			CommandList->SetPipelineState(m_Context.PipelineManager->GetGraphicsPipeline(m_GraphicsPipelineKey));
			m_GraphicsPipelineStateChanged = false;
		}
		if (m_DefaultDynamicStateSet) return;

		CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_DefaultDynamicStateSet = true;
	}

}