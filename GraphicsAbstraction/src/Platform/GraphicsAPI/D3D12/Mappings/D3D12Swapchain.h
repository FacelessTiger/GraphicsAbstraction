#pragma once

#include <GraphicsAbstraction/Renderer/Swapchain.h>
#include <GraphicsAbstraction/Renderer/Image.h>
#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Context.h>
#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Image.h>
#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Fence.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace GraphicsAbstraction {

	class D3D12Swapchain : public Swapchain
	{
	public:
		Microsoft::WRL::ComPtr<IDXGISwapChain4> Swapchain;
		DXGI_FORMAT ImageFormat;
		std::vector<Ref<D3D12Image>> Images;

		uint32_t ImageIndex = 0;

		uint32_t Width, Height;
		bool Vsync;
	public:
		D3D12Swapchain(const Ref<Window>& window, const glm::vec2& size, bool enableVSync);

		void Resize(uint32_t width, uint32_t height) override { }
		void SetVsync(bool enabled) override { Vsync = enabled; }

		Ref<Image> GetCurrent() override { return Images[ImageIndex]; }
	private:
		void CreateSwapchain(HWND hwnd);
		bool CheckTearingSupport();
	private:
		D3D12Context& m_Context;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RTVHeap;
	};

}