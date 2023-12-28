#include "bindless.hlsl"

struct PushConstant
{
	float2 scale;
	float2 offset;
	ArrayBuffer vertices;
	Texture texture;
	Sampler sampler;
};
PushConstant(PushConstant, pushConstants);

struct VertexInput
{
	float4 color: COLOR0;
	float2 uv: TEXCOORD0;
};

float4 main(VertexInput input): SV_Target
{
	return input.color * pushConstants.texture.Sample2D<float4>(pushConstants.sampler, input.uv);
}