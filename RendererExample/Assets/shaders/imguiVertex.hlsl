#include "bindless.hlsl"

struct ImDrawVert
{
	float2 pos;
	float2 uv;
	uint color;
};

struct PushConstant
{
	float2 scale;
	float2 offset;
	uint vertices;
	uint texture;
	uint sampler;
	uint vertexOffset;
};
PushConstant(PushConstant, pushConstants);

struct VertexOutput
{
	float4 position: SV_Position;
	float4 color: COLOR0;
	float2 uv: TEXCOORD0;
};

VertexOutput main(uint vertexID: SV_VertexID)
{
	ImDrawVert vertex = ArrayBuffer::Create(pushConstants.vertices).Load<ImDrawVert>(vertexID + pushConstants.vertexOffset);

	VertexOutput output;
	output.position = float4((vertex.pos * pushConstants.scale) + pushConstants.offset, 0, 1);
	output.position.y *= -1;
	output.color = UnpackUnorm4x8(vertex.color);
	output.uv = vertex.uv;
	return output;
}