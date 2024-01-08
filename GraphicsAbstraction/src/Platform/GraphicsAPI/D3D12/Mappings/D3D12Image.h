#pragma once

#include <GraphicsAbstraction/Renderer/Image.h>
#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Context.h>
#include <Platform/GraphicsAPI/D3D12/InternalManagers/D3D12ResourceHandle.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace GraphicsAbstraction {

	class D3D12Image : public Image
	{
	public:
		Microsoft::WRL::ComPtr<ID3D12Resource> Image;
		D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle;
		D3D12ResourceHandle Handle;

		D3D12_RESOURCE_STATES State;
		ImageFormat Format;
		ImageUsage Usage;
		uint32_t Width, Height;
	public:
		D3D12Image(const glm::vec2& size, ImageFormat format, ImageUsage usage);
		D3D12Image(Microsoft::WRL::ComPtr<ID3D12Resource> image, D3D12_RESOURCE_STATES state, ImageFormat format, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle);
		virtual ~D3D12Image();

		void Resize(const glm::vec2& size) override;

		inline uint32_t GetSampledHandle() const override { return Handle.GetValue(); }
		inline uint32_t GetStorageHandle() const override { return Handle.GetValue(); }
		inline uint32_t GetWidth() const override { return Width; }
		inline uint32_t GetHeight() const override { return Height; }
		inline glm::vec2 GetSize() const override { return glm::vec2(Width, Height); }

		void TransitionState(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_RESOURCE_STATES newState);
	private:
		void Create();
		void CreateViews(DXGI_FORMAT d3d12Format);
	private:
		Ref<D3D12Context> m_Context;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_Heap; // only used for depthstencil and color attachment
	};

}