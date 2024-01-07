#pragma once

#include <GraphicsAbstraction/Renderer/Sampler.h>
#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Context.h>
#include <Platform/GraphicsAPI/D3D12/InternalManagers/D3D12ResourceHandle.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace GraphicsAbstraction {

	class D3D12Sampler : public Sampler
	{
	public:
		D3D12ResourceHandle Handle = { ResourceType::Sampler };
	public:
		D3D12Sampler(Filter min, Filter mag);

		uint32_t GetHandle() const override { return Handle.GetValue(); }
	private:
		D3D12Context& m_Context;
	};

}