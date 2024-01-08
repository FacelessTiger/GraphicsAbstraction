#include "D3D12Fence.h"

namespace GraphicsAbstraction {

	Ref<Fence> Fence::Create()
	{
		return CreateRef<D3D12Fence>();
	}

	D3D12Fence::D3D12Fence()
		: m_Context(D3D12Context::GetReference())
	{
		m_Context->Device->CreateFence(Value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence));

		m_Event = CreateEvent(nullptr, false, false, nullptr);
		GA_CORE_ASSERT(m_Event, "Failed to create fence event");
	}

	D3D12Fence::~D3D12Fence()
	{
		CloseHandle(m_Event);
		m_Context->GetFrameDeletionQueue().Push(Fence);
	}

	void D3D12Fence::Wait()
	{
		Fence->SetEventOnCompletion(Value, m_Event);
		if (Fence->GetCompletedValue() < Value) WaitForSingleObject(m_Event, UINT32_MAX);
	}

}