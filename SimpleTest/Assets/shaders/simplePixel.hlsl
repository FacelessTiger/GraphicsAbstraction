#include "bindless.hlsl"

struct PushConstant
{
	float4x4 projection;
	uint vertices;
	uint image;
	uint sampler;
};
PushConstant(PushConstant, pushConstants);

struct VertexInput
{
	float4 position: SV_Position;
	float2 uv: TEXCOORD0;
};

float4 main(VertexInput input): SV_Target
{
	return Texture::Create(pushConstants.image).Sample2D<float4>(pushConstants.sampler, input.uv);
}