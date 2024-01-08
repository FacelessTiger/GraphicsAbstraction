#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include <vector>

namespace GraphicsAbstraction {

	class D3D12Context;

	class D3D12DeletionQueue
	{
	public:
		D3D12DeletionQueue(D3D12Context& context);
		virtual ~D3D12DeletionQueue();

		inline void Push(Microsoft::WRL::ComPtr<ID3D12Resource> resource) { m_Resources.push_back(resource); }
		inline void Push(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator) { m_Allocators.push_back(allocator); }
		inline void Push(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> list) { m_CommandLists.push_back(list); }
		inline void Push(Microsoft::WRL::ComPtr<ID3D12Fence> fence) { m_Fences.push_back(fence); }
		inline void Push(Microsoft::WRL::ComPtr<IDXGISwapChain4> swapchain) { m_Swapchains.push_back(swapchain); }

		void Flush();
	private:
		D3D12Context& m_Context;

		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_Resources;
		std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> m_Allocators;
		std::vector<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>> m_CommandLists;
		std::vector<Microsoft::WRL::ComPtr<ID3D12Fence>> m_Fences;
		std::vector<Microsoft::WRL::ComPtr<IDXGISwapChain4>> m_Swapchains;
	};

}