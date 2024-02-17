#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3D12MemAlloc.h>
#include <wrl.h>

#include <GraphicsAbstraction/Renderer/Image.h>

namespace GraphicsAbstraction::Utils {

	using namespace Microsoft::WRL;

	struct AllocatedResource
	{
		ComPtr<D3D12MA::Allocation> Allocation;
		ComPtr<ID3D12Resource> Resource;
	};

	DXGI_FORMAT GAImageFormatToD3D12(ImageFormat format);

}