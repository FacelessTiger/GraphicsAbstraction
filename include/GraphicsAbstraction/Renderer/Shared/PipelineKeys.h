#pragma once

#include <xxhash.h>

#include <vector>
#include <array>
#include <unordered_map>

#include <GraphicsAbstraction/Renderer/Image.h>
#include <GraphicsAbstraction/Renderer/CommandList.h>

#define GA_PIPELINE_EQUALITY(name) bool operator==(const name& other) const = default
#define GA_PIPELINE_HASH(name)															\
	template<>																			\
	struct hash<GraphicsAbstraction::name>												\
	{																					\
		std::size_t operator()(const GraphicsAbstraction::name& key) const				\
		{																				\
			return (std::size_t)XXH64(&key, sizeof(GraphicsAbstraction::name), 0);		\
		}																				\
	}

namespace GraphicsAbstraction {

	struct BlendInfo
	{
		bool BlendEnable = false;
		Blend SrcBlend = Blend::Zero;
		Blend DstBlend = Blend::Zero;
		Blend SrcBlendAlpha = Blend::Zero;
		Blend DstBlendAlpha = Blend::Zero;
		BlendOp BlendOpAlpha = BlendOp::Add;
		BlendOp BlendOp = BlendOp::Add;

		GA_PIPELINE_EQUALITY(BlendInfo);
	};

	struct ColorAttachment
	{
		ImageFormat Format;
		BlendInfo BlendInfo;

		GA_PIPELINE_EQUALITY(ColorAttachment);
	};

	struct GraphicsPipelineKey
	{
		// attachments/shaders
		std::array<uint32_t, 5> Shaders = {};
		std::array<ColorAttachment, 8> ColorAttachments = {};
		ImageFormat DepthAttachment = ImageFormat::Unknown;

		// depth test
		bool DepthTestEnable = false;
		bool DepthWriteEnable = false;
		CompareOperation DepthCompareOp = CompareOperation::Never;

		// Rasterization
		FillMode FillMode = FillMode::Solid;

		// vulkan specific
		void* Renderpass = nullptr;

		GA_PIPELINE_EQUALITY(GraphicsPipelineKey);
	};

	struct ComputePipelineKey
	{
		uint32_t Shader = 0;

		GA_PIPELINE_EQUALITY(ComputePipelineKey);
	};

}

namespace std {

	GA_PIPELINE_HASH(GraphicsPipelineKey);
	GA_PIPELINE_HASH(ComputePipelineKey);

}

#undef GA_PIPELINE_EQUALITY
#undef GA_PIPELINE_HASH