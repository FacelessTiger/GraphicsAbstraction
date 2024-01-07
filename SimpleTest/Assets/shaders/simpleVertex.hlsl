#include "bindless.hlsl"

struct Vertex
{
	float2 position;
	float2 uv;
};

struct PushConstant
{
	float4x4 projection;
	uint vertices;
	uint image;
	uint sampler;
};
PushConstant(PushConstant, pushConstants);

struct VertexOutput
{
	float4 position: SV_Position;
	float2 uv: TEXCOORD0;
};

VertexOutput main(uint vertexID: SV_VertexID)
{
	Vertex vertex = ArrayBuffer::Create(pushConstants.vertices).Load<Vertex>(vertexID);

	VertexOutput output;
	output.position = mul(pushConstants.projection, float4(vertex.position, 0.0f, 1.0f));
	output.uv = vertex.uv;
	return output;
}