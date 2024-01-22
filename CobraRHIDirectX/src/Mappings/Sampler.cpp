#include <DirectXRHI.h>

namespace GraphicsAbstraction {

	namespace Utils {

		D3D12_FILTER GAFiltersToD3D12(Filter min, Filter mag)
		{
			if (min == Filter::Nearest	&& mag == Filter::Nearest)	return D3D12_FILTER_MIN_MAG_MIP_POINT;
			if (min == Filter::Linear	&& mag == Filter::Nearest)	return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
			if (min == Filter::Nearest	&& mag == Filter::Linear)	return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
			if (min == Filter::Linear	&& mag == Filter::Linear)	return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;

			assert(false && "Unknown filters!");
			return D3D12_FILTER_MIN_MAG_MIP_POINT;
		}

	}

	Ref<Sampler> Sampler::Create(Filter min, Filter mag)
	{
		auto sampler = CreateRef<Sampler>();
		sampler->impl = new Impl<Sampler>(min, mag);
		return sampler;
	}

	Sampler::~Sampler()
	{
		delete impl;
	}

	uint32_t Sampler::GetHandle() const 
	{ 
		return impl->Handle.GetValue();
	}

	Impl<Sampler>::Impl(Filter min, Filter mag)
		: Context(Impl<GraphicsContext>::Reference)
	{
		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(Context->BindlessSamplerHeap->GetCPUDescriptorHandleForHeapStart(), Handle.GetValue(), Context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER));
		D3D12_SAMPLER_DESC desc = {
			.Filter = Utils::GAFiltersToD3D12(min, mag),
			.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP
		};
		Context->Device->CreateSampler(&desc, handle);
	}

}