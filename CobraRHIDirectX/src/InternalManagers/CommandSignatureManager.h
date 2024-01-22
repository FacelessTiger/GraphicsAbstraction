#pragma once

#include <InternalManagers/CommandSignatureKey.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace GraphicsAbstraction {

	template<typename T>
	struct Impl;
	struct GraphicsContext;

	class CommandSignatureManager
	{
	public:
		CommandSignatureManager(Impl<GraphicsContext>& context);
		~CommandSignatureManager();

		ID3D12CommandSignature* GetCommandSignature(const CommandSignatureKey& key);
	private:
		std::unordered_map<CommandSignatureKey, Microsoft::WRL::ComPtr<ID3D12CommandSignature>> m_CommandSignatures;

		Impl<GraphicsContext>& m_Context;
	};

}