#include <DirectXRHI.h>

namespace GraphicsAbstraction {

	Ref<Fence> Fence::Create()
	{
		auto fence = CreateRef<Fence>();
		fence->impl = new Impl<Fence>();
		return fence;
	}

	Fence::~Fence()
	{
		CloseHandle(impl->Event);
		impl->Context->GetFrameDeletionQueue().Push(impl->Fence);

		delete impl;
	}

	void Fence::Wait() const
	{
		impl->Fence->SetEventOnCompletion(impl->Value, impl->Event);
		if (impl->Fence->GetCompletedValue() < impl->Value) WaitForSingleObject(impl->Event, UINT32_MAX);
	}

	Impl<Fence>::Impl()
		: Context(Impl<GraphicsContext>::Reference)
	{
		Context->Device->CreateFence(Value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence));

		Event = CreateEvent(nullptr, false, false, nullptr);
		assert(Event && "Failed to create fence event");
	}

}