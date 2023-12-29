#include "bindless.hlsl"

struct Vertex
{
	float3 position;
	float uvX;
	float3 normal;
	float uvY;
	float4 color;
};

struct PushConstant
{
	float4x4 projection;
	ArrayBuffer vertices;
};
PushConstant(PushConstant, pushConstants);

struct VertexOutput
{
	float4 position: SV_Position;
	float3 color: COLOR0;
	float2 uv: TEXCOORD0;
};

VertexOutput main(uint vertexID: SV_VertexID)
{
	Vertex vertex = pushConstants.vertices.Load<Vertex>(vertexID);

	VertexOutput output;
	output.position = mul(pushConstants.projection, float4(vertex.position, 1.0f));
	output.color = vertex.color.xyz;
	output.uv = float2(vertex.uvX, vertex.uvY);
	return output;
}