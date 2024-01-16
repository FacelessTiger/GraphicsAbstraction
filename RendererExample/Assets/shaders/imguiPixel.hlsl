#include "Cobra.hlsl"

struct PushConstant
{
	float2 scale;
	float2 offset;
	uint vertices;
	uint texture;
	uint sampler;
};
PushConstant(PushConstant, pushConstants);

struct VertexInput
{
	float4 position: SV_Position;
	float4 color: COLOR0;
	float2 uv: TEXCOORD0;
};

float4 main(VertexInput input): SV_Target
{
	return input.color * Cobra::Texture::Create(pushConstants.texture).Sample2D<float4>(pushConstants.sampler, input.uv);
}