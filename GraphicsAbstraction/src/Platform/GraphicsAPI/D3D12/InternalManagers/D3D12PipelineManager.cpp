#include "D3D12PipelineManager.h"

#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Context.h>
#include <Platform/GraphicsAPI/D3D12/Mappings/D3D12Shader.h>
#include <Platform/GraphicsAPI/D3D12/InternalManagers/D3D12Utils.h>

#include <GraphicsAbstraction/Debug/Instrumentor.h>

namespace GraphicsAbstraction {

	namespace Utils {

		D3D12_COMPARISON_FUNC GACompareOpToD3D12(CompareOperation op)
		{
			switch (op)
			{
				case CompareOperation::Never:			return D3D12_COMPARISON_FUNC_NEVER;
				case CompareOperation::GreaterEqual:	return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
				case CompareOperation::LesserEqual:		return D3D12_COMPARISON_FUNC_LESS_EQUAL;
			}

			GA_CORE_ASSERT(false, "Unknown compare op!");
			return D3D12_COMPARISON_FUNC_NEVER;
		}

	}

	using namespace Microsoft::WRL;

	D3D12PipelineManager::D3D12PipelineManager(D3D12Context& context)
		: m_Context(context)
	{
	}

	D3D12PipelineManager::~D3D12PipelineManager()
	{
	}

	ID3D12PipelineState* D3D12PipelineManager::GetGraphicsPipeline(const GraphicsPipelineKey& key)
	{
		GA_PROFILE_SCOPE();
		if (m_GraphicsPipelines.find(key) != m_GraphicsPipelines.end()) return m_GraphicsPipelines[key].Get();

		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {
			.pRootSignature = m_Context.BindlessRootSignature.Get(),
			.VS = D3D12Shader::GetShaderByID(key.Shaders[0])->Shader,
			.PS = D3D12Shader::GetShaderByID(key.Shaders[4])->Shader,
			.SampleMask = (UINT)~0,
			.RasterizerState = {
				.FillMode = D3D12_FILL_MODE_SOLID,
				.CullMode = D3D12_CULL_MODE_NONE,
				.DepthClipEnable = true
			},
			.DepthStencilState = {
				.DepthEnable = key.DepthTestEnable,
				.DepthWriteMask = key.DepthWriteEnable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO,
				.DepthFunc = Utils::GACompareOpToD3D12(key.DepthCompareOp)
			},
			.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
			.DSVFormat = DXGI_FORMAT_D32_FLOAT,
			.SampleDesc = {
				.Count = 1,
				.Quality = 0
			},
		};

		for (int i = 0; i < key.ColorAttachments.size(); i++)
		{
			if (key.ColorAttachments[i] == ImageFormat::Unknown) break;

			desc.RTVFormats[i] = Utils::GAImageFormatToD3D12(key.ColorAttachments[i]);
			desc.BlendState.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
			desc.NumRenderTargets++;
		}

		ComPtr<ID3D12PipelineState> pipeline;
		D3D12_CHECK(m_Context.Device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipeline)));
		m_GraphicsPipelines[key] = pipeline;

		return pipeline.Get();
	}

}