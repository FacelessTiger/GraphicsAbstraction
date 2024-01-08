#pragma once

#include <GraphicsAbstraction/Renderer/Fence.h>
#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Context.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace GraphicsAbstraction {

	class D3D12Fence : public Fence
	{
	public:
		Microsoft::WRL::ComPtr<ID3D12Fence> Fence;
		uint64_t Value = 0;
	public:
		D3D12Fence();
		virtual ~D3D12Fence();

		void Wait();
	private:
		Ref<D3D12Context> m_Context;
		HANDLE m_Event;
	};

}