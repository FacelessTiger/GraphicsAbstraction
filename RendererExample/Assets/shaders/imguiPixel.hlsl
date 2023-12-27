#define PushConstants	[[vk::offset(16)]] float2 scale; \
						[[vk::offset(24)]] float2 offset; \
						[[vk::offset(32)]] Texture texture; \
						[[vk::offset(36)]] Sampler sampler;
#include "bindless.hlsl"

struct VertexInput
{
	float4 color: COLOR0;
	float2 uv: TEXCOORD0;
};

float4 main(VertexInput input): SV_Target
{
	return input.color * g_PushConstants.texture.Sample2D<float4>(g_PushConstants.sampler, input.uv);
}