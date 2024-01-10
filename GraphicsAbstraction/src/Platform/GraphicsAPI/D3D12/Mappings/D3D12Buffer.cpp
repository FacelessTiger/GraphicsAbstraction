#include "D3D12Buffer.h"

#include <d3dx12/d3dx12.h>

namespace GraphicsAbstraction {

	Ref<Buffer> Buffer::Create(uint32_t size, BufferUsage usage, BufferFlags flags)
	{
		return CreateRef<D3D12Buffer>(size, usage, flags);
	}

	D3D12Buffer::D3D12Buffer(uint32_t size, BufferUsage usage, BufferFlags flags)
		: m_Context(D3D12Context::GetReference()), Size(size), Usage(usage)
	{
		D3D12_HEAP_TYPE heapType;
		if (flags & BufferFlags::DeviceLocal) heapType = (flags & BufferFlags::Mapped) ? D3D12_HEAP_TYPE_GPU_UPLOAD : D3D12_HEAP_TYPE_DEFAULT;
		else heapType = D3D12_HEAP_TYPE_UPLOAD;
		auto heapProperties = CD3DX12_HEAP_PROPERTIES(heapType);

		auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
		D3D12_CHECK(m_Context->Device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&Resource)));
		
		if (flags & BufferFlags::Mapped)
		{
			D3D12_RANGE range = {
				.Begin = 0,
				.End = 0
			};
			D3D12_CHECK(Resource->Map(0, &range, &Data));
		}

		if (usage & BufferUsage::StorageBuffer)
		{
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_Context->BindlessDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), Handle.GetValue(), m_Context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

			D3D12_SHADER_RESOURCE_VIEW_DESC shaderViewDesc = {
				.Format = DXGI_FORMAT_R32_TYPELESS,
				.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Buffer = {
					.NumElements = size / 4,
					.Flags = D3D12_BUFFER_SRV_FLAG_RAW
				}
			};
			m_Context->Device->CreateShaderResourceView(Resource.Get(), &shaderViewDesc, handle);
		}
	}

	D3D12Buffer::~D3D12Buffer()
	{
		m_Context->GetFrameDeletionQueue().Push(Resource);
	}

	void D3D12Buffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		uint32_t bufferSize = size ? size : Size; // If user passes a size use it, otherwise use creation size
		memcpy((char*)Data + offset, data, bufferSize);
	}

	void D3D12Buffer::TransitionState(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_RESOURCE_STATES newState)
	{
		if (State == newState) return;
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(Resource.Get(), State, newState, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

		commandList->ResourceBarrier(1, &barrier);
		State = newState;
	}

}