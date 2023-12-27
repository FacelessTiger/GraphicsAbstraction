#define PushConstants	[[vk::offset(16)]] float2 scale; \
						[[vk::offset(24)]] float2 offset;
#include "bindless.hlsl"

struct ImDrawVert
{
	float2 pos;
	float2 uv;
	uint color;
};

struct Bindings
{
	ArrayBuffer vertices;
};

struct VertexOutput
{
	float4 position: SV_Position;
	float4 color: COLOR0;
	float2 uv: TEXCOORD0;
};

VertexOutput main(uint vertexID: SV_VertexID)
{
	Bindings bnd = loadBindings<Bindings>();
	ImDrawVert vertex = bnd.vertices.Load<ImDrawVert>(vertexID, 20);

	VertexOutput output;
	output.position = float4((vertex.pos * g_PushConstants.scale) + g_PushConstants.offset, 0, 1);
	output.color = UnpackUnorm4x8(vertex.color);
	output.uv = vertex.uv;
	return output;
}