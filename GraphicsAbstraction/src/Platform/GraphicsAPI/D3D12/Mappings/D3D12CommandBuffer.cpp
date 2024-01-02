#include "D3D12CommandBuffer.h"

#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Image.h>
#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Swapchain.h>

#include <glm/gtc/type_ptr.hpp>

namespace GraphicsAbstraction {

	using namespace Microsoft::WRL;

	D3D12CommandBuffer::D3D12CommandBuffer(ComPtr<ID3D12GraphicsCommandList> commandList)
		: CommandList(commandList)
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

}