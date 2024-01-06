#pragma once

#include <Platform/GraphicsAPI/Shared/PipelineKeys.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace GraphicsAbstraction {

	class D3D12Context;

	class D3D12PipelineManager
	{
	public:
		D3D12PipelineManager(D3D12Context& context);
		~D3D12PipelineManager();

		ID3D12PipelineState* GetGraphicsPipeline(const GraphicsPipelineKey& key);
	private:
		std::unordered_map<GraphicsPipelineKey, Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_GraphicsPipelines;

		D3D12Context& m_Context;
	};

}