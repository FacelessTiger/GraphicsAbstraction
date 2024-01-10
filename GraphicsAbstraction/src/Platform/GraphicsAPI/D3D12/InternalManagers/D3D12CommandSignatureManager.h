#pragma once

#include <Platform/GraphicsAPI/D3D12/InternalManagers/D3D12CommandSignatureKey.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace GraphicsAbstraction {

	class D3D12Context;

	class D3D12CommandSignatureManager
	{
	public:
		D3D12CommandSignatureManager(D3D12Context& context);
		~D3D12CommandSignatureManager();

		ID3D12CommandSignature* GetCommandSignature(const D3D12CommandSignatureKey& key);
	private:
		std::unordered_map<D3D12CommandSignatureKey, Microsoft::WRL::ComPtr<ID3D12CommandSignature>> m_CommandSignatures;

		D3D12Context& m_Context;
	};

}