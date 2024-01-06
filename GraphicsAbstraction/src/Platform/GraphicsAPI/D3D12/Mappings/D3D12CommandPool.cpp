#include "D3D12CommandPool.h"

#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Queue.h>
#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12CommandBuffer.h>

namespace GraphicsAbstraction {

	Ref<CommandPool> CommandPool::Create(const Ref<Queue>& queue)
	{
		return CreateRef<D3D12CommandPool>(queue);
	}

	D3D12CommandPool::D3D12CommandPool(const Ref<Queue>& queue)
		: m_Context(D3D12Context::GetReference())
	{
		auto& d3d12Queue = (D3D12Queue&)*queue;

		D3D12_CHECK(m_Context.Device->CreateCommandAllocator(d3d12Queue.Type, IID_PPV_ARGS(&Allocator)));

		D3D12_CHECK(m_Context.Device->CreateCommandList(0, d3d12Queue.Type, Allocator.Get(), nullptr, IID_PPV_ARGS(&MainCommandList)));
		D3D12_CHECK(MainCommandList->Close());
	}

	CommandPool* D3D12CommandPool::Reset()
	{
		Allocator->Reset();
		MainCommandList->Reset(Allocator.Get(), nullptr);

		return this;
	}

	Ref<CommandBuffer> D3D12CommandPool::Begin() const
	{
		auto descriptorHeap = m_Context.BindlessDescriptorHeap.Get();
		MainCommandList->SetGraphicsRootSignature(m_Context.BindlessRootSignature.Get());
		MainCommandList->SetDescriptorHeaps(1, &descriptorHeap);

		return CreateRef<D3D12CommandBuffer>(MainCommandList);
	}

}