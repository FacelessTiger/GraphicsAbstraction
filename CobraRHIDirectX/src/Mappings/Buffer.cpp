#include <DirectXRHI.h>

namespace GraphicsAbstraction {

	Ref<Buffer> Buffer::Create(uint32_t size, BufferUsage usage, BufferFlags flags)
	{
		auto buffer = CreateRef<Buffer>();
		buffer->impl = new Impl<Buffer>(size, usage, flags);
		return buffer;
	}

	Buffer::~Buffer()
	{
		impl->Context->GetFrameDeletionQueue().Push(impl->AResource);
		delete impl;
	}

	void Buffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		uint32_t bufferSize = size ? size : impl->Size; // If user passes a size use it, otherwise use creation size
		memcpy((char*)impl->Data + offset, data, bufferSize);
	}

	void Buffer::SetData(const Ref<Buffer>& buffer) 
	{

	}

	void Buffer::GetData(void* data, uint32_t size, uint32_t offset) { memcpy(data, (char*)impl->Data + offset, size); }
	uint32_t Buffer::GetHandle() const { return impl->Handle.GetValue(); }
	uint32_t Buffer::GetSize() const { return impl->Size; }

	Impl<Buffer>::Impl(uint32_t size, BufferUsage usage, BufferFlags flags)
		: Context(Impl<GraphicsContext>::Reference), Size(size), Usage(usage)
	{
		D3D12_HEAP_TYPE heapType;
		if (flags & BufferFlags::DeviceLocal)
		{
			if (flags & BufferFlags::Mapped) heapType = D3D12_HEAP_TYPE_GPU_UPLOAD;
			else heapType = D3D12_HEAP_TYPE_DEFAULT;
		}
		else
		{
			if (usage & BufferUsage::TransferDst) heapType = D3D12_HEAP_TYPE_READBACK;
			else heapType = D3D12_HEAP_TYPE_UPLOAD;
		}
		D3D12MA::ALLOCATION_DESC allocDesc = { .HeapType = heapType };

		auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(size, (usage & BufferUsage::IndirectBuffer) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE);
		D3D12_CHECK(Context->Allocator->CreateResource(&allocDesc, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, &AResource.Allocation, IID_PPV_ARGS(&AResource.Resource)));
		
		if (flags & BufferFlags::Mapped)
		{
			D3D12_RANGE range = {
				.Begin = 0,
				.End = 0
			};
			D3D12_CHECK(AResource.Resource->Map(0, &range, &Data));
		}

		if (usage & BufferUsage::StorageBuffer)
		{
			uint32_t offsetSize = Context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(Context->BindlessDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), Handle.GetValue(), offsetSize);

			D3D12_SHADER_RESOURCE_VIEW_DESC shaderViewDesc = {
				.Format = DXGI_FORMAT_R32_TYPELESS,
				.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Buffer = {
					.NumElements = size / 4,
					.Flags = D3D12_BUFFER_SRV_FLAG_RAW
				}
			};
			Context->Device->CreateShaderResourceView(AResource.Resource.Get(), &shaderViewDesc, handle);

			if (usage & BufferUsage::IndirectBuffer)
			{
				handle.Offset(1, offsetSize);
				D3D12_UNORDERED_ACCESS_VIEW_DESC unorderedViewDesc = {
					.Format = DXGI_FORMAT_R32_TYPELESS,
					.ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
					.Buffer = {
						.NumElements = size / 4,
						.Flags = D3D12_BUFFER_UAV_FLAG_RAW
					}
				};
				Context->Device->CreateUnorderedAccessView(AResource.Resource.Get(), nullptr, &unorderedViewDesc, handle);
			}
		}
	}

	void Impl<Buffer>::TransitionState(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_RESOURCE_STATES newState)
	{
		if (State == newState) return;
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(AResource.Resource.Get(), State, newState, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

		commandList->ResourceBarrier(1, &barrier);
		State = newState;
	}

}