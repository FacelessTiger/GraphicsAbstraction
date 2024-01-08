#include "D3D12Sampler.h"

#include <d3dx12/d3dx12.h>

namespace GraphicsAbstraction {

	namespace Utils {

		D3D12_FILTER GAFiltersToD3D12(Filter min, Filter mag)
		{
			if (min == Filter::Nearest	&& mag == Filter::Nearest)	return D3D12_FILTER_MIN_MAG_MIP_POINT;
			if (min == Filter::Linear	&& mag == Filter::Nearest)	return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
			if (min == Filter::Nearest	&& mag == Filter::Linear)	return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
			if (min == Filter::Linear	&& mag == Filter::Linear)	return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;

			GA_CORE_ASSERT(false, "Unknown filters!");
			return D3D12_FILTER_MIN_MAG_MIP_POINT;
		}

	}

	Ref<Sampler> Sampler::Create(Filter min, Filter mag)
	{
		return CreateRef<D3D12Sampler>(min, mag);
	}

	D3D12Sampler::D3D12Sampler(Filter min, Filter mag)
		: m_Context(D3D12Context::GetReference())
	{
		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_Context->BindlessSamplerHeap->GetCPUDescriptorHandleForHeapStart(), Handle.GetValue(), m_Context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER));
		D3D12_SAMPLER_DESC desc = {
			.Filter = Utils::GAFiltersToD3D12(min, mag),
			.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP
		};
		m_Context->Device->CreateSampler(&desc, handle);
	}

}