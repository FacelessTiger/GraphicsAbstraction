#include "Cobra.hlsl"

struct PushConstant
{
	float4x4 projection;
	ArrayBuffer quadData;
	Sampler sampler;
};
PushConstant(PushConstant, pushConstants);

struct VertexInput
{
	float4 position: SV_Position;
	float4 color: COLOR0;
	Texture texture: BLENDINDICES0;
	float2 uv: TEXCOORD0;
};

float4 main(VertexInput input): SV_Target
{
	return input.color * input.texture.Sample2D<float4>(pushConstants.sampler, input.uv);
}