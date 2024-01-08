#pragma once

#include <GraphicsAbstraction/Renderer/CommandPool.h>
#include <GraphicsAbstraction/Renderer/CommandBuffer.h>
#include <GraphicsAbstraction/Renderer/Queue.h>

#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Context.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace GraphicsAbstraction {

	class D3D12CommandPool : public CommandPool
	{
	public:
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> Allocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> MainCommandList;
	public:
		D3D12CommandPool(const Ref<Queue>& queue);
		virtual ~D3D12CommandPool();

		CommandPool* Reset() override;
		Ref<CommandBuffer> Begin() const override;
	private:
		Ref<D3D12Context> m_Context;
	};

}