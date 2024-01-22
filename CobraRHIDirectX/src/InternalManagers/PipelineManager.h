#pragma once

#include <GraphicsAbstraction/Renderer/Shared/PipelineKeys.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace GraphicsAbstraction {

	using namespace Microsoft::WRL;

	class PipelineManager
	{
	public:
		PipelineManager(Impl<GraphicsContext>& context);
		~PipelineManager();

		ID3D12PipelineState* GetGraphicsPipeline(const GraphicsPipelineKey& key);
		ID3D12PipelineState* GetComputePipeline(const ComputePipelineKey& key);
	private:
		std::unordered_map<GraphicsPipelineKey, ComPtr<ID3D12PipelineState>> m_GraphicsPipelines;
		std::unordered_map<ComputePipelineKey, ComPtr<ID3D12PipelineState>> m_ComputePipelines;

		Impl<GraphicsContext>& m_Context;
	};

}