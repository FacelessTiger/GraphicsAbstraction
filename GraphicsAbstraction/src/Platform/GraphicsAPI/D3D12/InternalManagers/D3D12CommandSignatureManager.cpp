#include "D3D12CommandSignatureManager.h"

#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Context.h>
#include <GraphicsAbstraction/Debug/Instrumentor.h>

namespace GraphicsAbstraction {

	using namespace Microsoft::WRL;

	D3D12CommandSignatureManager::D3D12CommandSignatureManager(D3D12Context& context)
		: m_Context(context)
	{ }

	D3D12CommandSignatureManager::~D3D12CommandSignatureManager()
	{ }

	ID3D12CommandSignature* D3D12CommandSignatureManager::GetCommandSignature(const D3D12CommandSignatureKey& key)
	{
		GA_PROFILE_SCOPE();
		if (m_CommandSignatures.find(key) != m_CommandSignatures.end()) return m_CommandSignatures[key].Get();

		D3D12_INDIRECT_ARGUMENT_DESC args[2];
		args[0] = {
			.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT,
			.Constant = {
				.RootParameterIndex = 1,
				.Num32BitValuesToSet = 2
			}
		};
		args[1] = { .Type = key.DrawType };

		D3D12_COMMAND_SIGNATURE_DESC desc = {
			.ByteStride = key.Stride,
			.NumArgumentDescs = 2,
			.pArgumentDescs = args
		};

		ComPtr<ID3D12CommandSignature> commandSignature;
		D3D12_CHECK(m_Context.Device->CreateCommandSignature(&desc, m_Context.BindlessRootSignature.Get(), IID_PPV_ARGS(&commandSignature)));
		m_CommandSignatures[key] = commandSignature;

		return commandSignature.Get();
	}

}