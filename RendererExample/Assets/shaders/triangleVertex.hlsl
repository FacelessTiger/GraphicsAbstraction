#include "bindless.hlsl"

struct Vertex
{
	float3 position;
};

struct PushConstant
{
	float4x4 projection;
	ArrayBuffer vertices;
};
PushConstant(PushConstant, pushConstants);

float4 main(uint vertexID: SV_VertexID): SV_Position
{
	Vertex vertex = pushConstants.vertices.Load<Vertex>(vertexID);

	return mul(pushConstants.projection, float4(vertex.position, 1.0f));
}