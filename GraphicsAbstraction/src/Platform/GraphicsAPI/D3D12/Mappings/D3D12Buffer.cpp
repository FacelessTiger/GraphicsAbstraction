#include "D3D12Buffer.h"

#include <d3dx12/d3dx12.h>

namespace GraphicsAbstraction {

	namespace Utils {

		D3D12_RESOURCE_STATES GABufferUsageToD3D12(BufferUsage usage)
		{
			D3D12_RESOURCE_STATES ret = D3D12_RESOURCE_STATE_COMMON;

			if (usage & BufferUsage::StorageBuffer)	ret |= D3D12_RESOURCE_STATE_GENERIC_READ;
			if (usage & BufferUsage::TransferSrc)	ret |= D3D12_RESOURCE_STATE_COPY_SOURCE;
			if (usage & BufferUsage::TransferDst)	ret |= D3D12_RESOURCE_STATE_COPY_DEST;
			if (usage & BufferUsage::IndexBuffer)	ret |= D3D12_RESOURCE_STATE_INDEX_BUFFER;

			return ret;
		}

	}

	Ref<Buffer> Buffer::Create(uint32_t size, BufferUsage usage, BufferFlags flags)
	{
		return CreateRef<D3D12Buffer>(size, usage, flags);
	}

	D3D12Buffer::D3D12Buffer(uint32_t size, BufferUsage usage, BufferFlags flags)
		: m_Context(D3D12Context::GetReference()), Size(size)
	{
		D3D12_HEAP_TYPE heapType;
		if (flags & BufferFlags::DeviceLocal) heapType = (flags & BufferFlags::Mapped) ? D3D12_HEAP_TYPE_GPU_UPLOAD : D3D12_HEAP_TYPE_DEFAULT;
		else heapType = D3D12_HEAP_TYPE_UPLOAD;
		auto heapProperties = CD3DX12_HEAP_PROPERTIES(heapType);

		D3D12_RESOURCE_ALLOCATION_INFO allocInfo = {
			.SizeInBytes = size,
			.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT
		};
		auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(allocInfo);

		D3D12_RESOURCE_STATES resourceState = (heapType == D3D12_HEAP_TYPE_DEFAULT) ? D3D12_RESOURCE_STATE_COMMON : Utils::GABufferUsageToD3D12(usage);
		D3D12_CHECK(m_Context.Device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, resourceState, nullptr, IID_PPV_ARGS(&Resource)));

		if (flags & BufferFlags::Mapped)
		{
			D3D12_RANGE range = {
				.Begin = 0,
				.End = size
			};
			D3D12_CHECK(Resource->Map(0, &range, &Data));
		}

		if (usage & BufferUsage::StorageBuffer)
		{
			auto descriptorSize = m_Context.Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_Context.BindlessDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), Handle.GetValue(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			D3D12_SHADER_RESOURCE_VIEW_DESC shaderViewDesc = {
				.Format = DXGI_FORMAT_R32_TYPELESS,
				.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Buffer = {
					.NumElements = size / 4,
					.Flags = D3D12_BUFFER_SRV_FLAG_RAW
				}
			};
			m_Context.Device->CreateShaderResourceView(Resource.Get(), &shaderViewDesc, handle);
		}
	}

	void D3D12Buffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		uint32_t bufferSize = size ? size : Size; // If user passes a size use it, otherwise use creation size
		memcpy((char*)Data + offset, data, bufferSize);
	}

}