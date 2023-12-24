#include "bindless.hlsl"

struct Vertex
{
	float3 position;
};

struct Bindings
{
	ArrayBuffer vertices;
};

float4 main(uint vertexID: SV_VertexID): SV_Position
{
	Bindings bnd = loadBindings<Bindings>();
	Vertex vertex = bnd.vertices.Load<Vertex>(vertexID);

	return float4(vertex.position, 1.0f);
}