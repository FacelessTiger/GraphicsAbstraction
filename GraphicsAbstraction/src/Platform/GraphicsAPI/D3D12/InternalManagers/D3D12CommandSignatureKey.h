#pragma once

#include <unordered_map>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <xxhash.h>

namespace GraphicsAbstraction {

	struct D3D12CommandSignatureKey
	{
		D3D12_INDIRECT_ARGUMENT_TYPE DrawType;
		uint32_t Stride;

		bool operator==(const D3D12CommandSignatureKey& other) const = default;
	};

}

namespace std {

	template<>
	struct hash<GraphicsAbstraction::D3D12CommandSignatureKey>
	{
		std::size_t operator()(const GraphicsAbstraction::D3D12CommandSignatureKey& key) const
		{
			return (std::size_t)XXH64(&key, sizeof(GraphicsAbstraction::D3D12CommandSignatureKey), 0);
		}
	};

}