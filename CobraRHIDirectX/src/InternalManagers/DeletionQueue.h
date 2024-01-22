#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include <vector>

namespace GraphicsAbstraction {

	using namespace Microsoft::WRL;

	template<typename T>
	struct Impl;
	struct GraphicsContext;

	class DeletionQueue
	{
	public:
		DeletionQueue(Impl<GraphicsContext>& context);
		virtual ~DeletionQueue();

		inline void Push(ComPtr<ID3D12Resource> resource) { m_Resources.push_back(resource); }
		inline void Push(ComPtr<ID3D12CommandAllocator> allocator) { m_Allocators.push_back(allocator); }
		inline void Push(ComPtr<ID3D12GraphicsCommandList> list) { m_CommandLists.push_back(list); }
		inline void Push(ComPtr<ID3D12Fence> fence) { m_Fences.push_back(fence); }
		inline void Push(ComPtr<IDXGISwapChain4> swapchain) { m_Swapchains.push_back(swapchain); }

		void Flush();
	private:
		Impl<GraphicsContext>& m_Context;

		std::vector<ComPtr<ID3D12Resource>> m_Resources;
		std::vector<ComPtr<ID3D12CommandAllocator>> m_Allocators;
		std::vector<ComPtr<ID3D12GraphicsCommandList>> m_CommandLists;
		std::vector<ComPtr<ID3D12Fence>> m_Fences;
		std::vector<ComPtr<IDXGISwapChain4>> m_Swapchains;
	};

}