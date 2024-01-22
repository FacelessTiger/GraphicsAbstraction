#include <DirectXRHI.h>

namespace GraphicsAbstraction {

	Ref<CommandAllocator> CommandAllocator::Create(const Ref<Queue>& queue)
	{
		auto allocator = CreateRef<CommandAllocator>();
		allocator->impl = new Impl<CommandAllocator>(queue);
		return allocator;
	}

	CommandAllocator::~CommandAllocator()
	{
		impl->Context->GetFrameDeletionQueue().Push(impl->Allocator);
		impl->Context->GetFrameDeletionQueue().Push(impl->MainCommandList);
		
		delete impl;
	}

	CommandAllocator* CommandAllocator::Reset()
	{
		impl->Allocator->Reset();
		impl->MainCommandList->Reset(impl->Allocator.Get(), nullptr);

		return this;
	}

	Ref<CommandList> CommandAllocator::Begin() const
	{
		ID3D12DescriptorHeap* descriptorHeap[2] = { impl->Context->BindlessDescriptorHeap.Get(), impl->Context->BindlessSamplerHeap.Get() };
		impl->MainCommandList->SetDescriptorHeaps(2, descriptorHeap);

		impl->MainCommandList->SetComputeRootSignature(impl->Context->BindlessRootSignature.Get());
		impl->MainCommandList->SetGraphicsRootSignature(impl->Context->BindlessRootSignature.Get());

		auto list = CreateRef<CommandList>();
		list->impl = new Impl<CommandList>(impl->MainCommandList);
		return list;
	}

	Impl<CommandAllocator>::Impl(const Ref<Queue>& queue)
		: Context(Impl<GraphicsContext>::Reference)
	{
		D3D12_CHECK(Context->Device->CreateCommandAllocator(queue->impl->Type, IID_PPV_ARGS(&Allocator)));

		D3D12_CHECK(Context->Device->CreateCommandList(0, queue->impl->Type, Allocator.Get(), nullptr, IID_PPV_ARGS(&MainCommandList)));
		D3D12_CHECK(MainCommandList->Close());
	}

}