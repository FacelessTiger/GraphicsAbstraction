#pragma once

#include <GraphicsAbstraction/Renderer/Buffer.h>
#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Context.h>
#include <Platform/GraphicsAPI/D3D12/InternalManagers/D3D12ResourceHandle.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace GraphicsAbstraction {

	class D3D12Buffer : public Buffer
	{
	public:
		Microsoft::WRL::ComPtr<ID3D12Resource> Resource;
		D3D12ResourceHandle Handle;
		D3D12_RESOURCE_STATES State = D3D12_RESOURCE_STATE_COMMON;

		void* Data = nullptr;
		uint32_t Size;
	public:
		D3D12Buffer(uint32_t size, BufferUsage usage, BufferFlags flags);
		virtual ~D3D12Buffer();

		void SetData(const void* data, uint32_t size = 0, uint32_t offset = 0) override;
		void SetData(const Ref<Buffer>& buffer) override { }

		void GetData(void* data, uint32_t size, uint32_t offset) override { }
		inline uint32_t GetHandle() const override { return Handle.GetValue(); }
		inline uint32_t GetSize() const override { return Size; }

		void TransitionState(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_RESOURCE_STATES newState);
	private:
		Ref<D3D12Context> m_Context;
	};

}