#define PushConstants	float4x4 projection; \
						Sampler sampler;
#include "bindless.hlsl"

struct VertexInput
{
	float4 color: COLOR0;
	Texture texture: BLENDINDICES0;
	float2 uv: TEXCOORD0;
};

float4 main(VertexInput input): SV_Target
{
	return input.color * input.texture.Sample2D<float4>(g_PushConstants.sampler, input.uv);
}