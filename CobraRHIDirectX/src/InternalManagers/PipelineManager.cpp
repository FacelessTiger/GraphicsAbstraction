#include "PipelineManager.h"

#include <DirectXRHI.h>

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

			assert(false && "Unknown compare op!");
			return D3D12_COMPARISON_FUNC_NEVER;
		}

		D3D12_BLEND GABlendToD3D12(Blend blend)
		{
			switch (blend)
			{
				case Blend::Zero:				return D3D12_BLEND_ZERO;
				case Blend::One:				return D3D12_BLEND_ONE;
				case Blend::SrcAlpha:			return D3D12_BLEND_SRC_ALPHA;
				case Blend::DstAlpha:			return D3D12_BLEND_DEST_ALPHA;
				case Blend::OneMinusSrcAlpha:	return D3D12_BLEND_INV_SRC_ALPHA;
			}

			assert(false && "Unknown blend!");
			return D3D12_BLEND_ZERO;
		}

		D3D12_BLEND_OP GABlendOpToD3D12(BlendOp op)
		{
			switch (op)
			{
				case BlendOp::Add: return D3D12_BLEND_OP_ADD;
			}

			assert(false && "Unknown blend op!");
			return D3D12_BLEND_OP_ADD;
		}

		D3D12_FILL_MODE GAFillModeToD3D12(FillMode mode)
		{
			switch (mode)
			{
				case FillMode::Solid:		return D3D12_FILL_MODE_SOLID;
				case FillMode::Wireframe:	return D3D12_FILL_MODE_WIREFRAME;
			}

			assert(false && "Unkown fill mode!");
			return D3D12_FILL_MODE_SOLID;
		}

	}

	PipelineManager::PipelineManager(Impl<GraphicsContext>& context)
		: m_Context(context)
	{ }

	PipelineManager::~PipelineManager()
	{ }

	ID3D12PipelineState* PipelineManager::GetGraphicsPipeline(const GraphicsPipelineKey& key)
	{
		if (m_GraphicsPipelines.find(key) != m_GraphicsPipelines.end()) return m_GraphicsPipelines[key].Get();

		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {
			.pRootSignature = m_Context.BindlessRootSignature.Get(),
			.VS = Impl<Shader>::GetShaderByID(key.Shaders[0])->Shader,
			.PS = Impl<Shader>::GetShaderByID(key.Shaders[4])->Shader,
			.SampleMask = (UINT)~0,
			.RasterizerState = {
				.FillMode = Utils::GAFillModeToD3D12(key.FillMode),
				.CullMode = D3D12_CULL_MODE_NONE,
				.DepthClipEnable = true
			},
			.DepthStencilState = {
				.DepthEnable = key.DepthTestEnable,
				.DepthWriteMask = key.DepthWriteEnable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO,
				.DepthFunc = Utils::GACompareOpToD3D12(key.DepthCompareOp)
			},
			.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
			.DSVFormat = Utils::GAImageFormatToD3D12(key.DepthAttachment),
			.SampleDesc = {
				.Count = 1,
				.Quality = 0
			},
		};

		D3D12_RENDER_TARGET_BLEND_DESC blendDesc = {
			.BlendEnable = key.BlendEnable,
			.SrcBlend = Utils::GABlendToD3D12(key.SrcBlend),
			.DestBlend = Utils::GABlendToD3D12(key.DstBlend),
			.BlendOp = Utils::GABlendOpToD3D12(key.BlendOp),
			.SrcBlendAlpha = Utils::GABlendToD3D12(key.SrcBlendAlpha),
			.DestBlendAlpha = Utils::GABlendToD3D12(key.DstBlendAlpha),
			.BlendOpAlpha = Utils::GABlendOpToD3D12(key.BlendOpAlpha),
			.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL
		};

		for (int i = 0; i < key.ColorAttachments.size(); i++)
		{
			if (key.ColorAttachments[i] == ImageFormat::Unknown) break;

			desc.RTVFormats[i] = Utils::GAImageFormatToD3D12(key.ColorAttachments[i]);
			desc.BlendState.RenderTarget[i] = blendDesc;
			desc.NumRenderTargets++;
		}

		ComPtr<ID3D12PipelineState> pipeline;
		D3D12_CHECK(m_Context.Device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipeline)));
		m_GraphicsPipelines[key] = pipeline;

		return pipeline.Get();
	}

	ID3D12PipelineState* PipelineManager::GetComputePipeline(const ComputePipelineKey& key)
	{
		if (m_ComputePipelines.find(key) != m_ComputePipelines.end()) return m_ComputePipelines[key].Get();

		D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {
			.pRootSignature = m_Context.BindlessRootSignature.Get(),
			.CS = Impl<Shader>::GetShaderByID(key.Shader)->Shader
		};

		ComPtr<ID3D12PipelineState> pipeline;
		D3D12_CHECK(m_Context.Device->CreateComputePipelineState(&desc, IID_PPV_ARGS(&pipeline)));
		m_ComputePipelines[key] = pipeline;

		return pipeline.Get();
	}

}