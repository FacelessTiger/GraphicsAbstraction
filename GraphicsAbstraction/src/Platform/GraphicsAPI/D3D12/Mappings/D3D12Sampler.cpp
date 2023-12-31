#include "D3D12Sampler.h"

namespace GraphicsAbstraction {

	Ref<Sampler> Sampler::Create(Filter min, Filter mag)
	{
		return CreateRef<D3D12Sampler>(min, mag);
	}

}