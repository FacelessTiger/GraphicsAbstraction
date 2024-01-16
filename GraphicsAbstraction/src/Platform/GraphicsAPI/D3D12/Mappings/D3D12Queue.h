#pragma once

#include <GraphicsAbstraction/Renderer/Queue.h>
#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Fence.h>

#include <d3d12.h>
#include <wrl.h>

namespace GraphicsAbstraction {

	class D3D12Queue : public Queue
	{
	public:
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> Queue;
		D3D12_COMMAND_LIST_TYPE Type;
	public:
		D3D12Queue(Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue, D3D12_COMMAND_LIST_TYPE type);
		~D3D12Queue();

		void Acquire(const Ref<Swapchain>& swapchain, const Ref<Fence>& fence) override;
		void Submit(const Ref<CommandList>& cmd, const Ref<Fence>& wait, const Ref<Fence>& signal);
		void Present(const Ref<Swapchain>& swapchain, const Ref<Fence>& wait) override;
	private:
		Ref<D3D12Fence> m_DeletionFence;
	};

}