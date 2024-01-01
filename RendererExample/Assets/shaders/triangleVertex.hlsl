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
	ArrayBuffer modelMatrices;
};
PushConstant(PushConstant, pushConstants);

struct VertexOutput
{
	float4 position: SV_Position;
	float3 color: COLOR0;
	float2 uv: TEXCOORD0;
};

VertexOutput main(uint vertexID: SV_VertexID, uint instanceIndex: SV_InstanceID)
{
	Vertex vertex = pushConstants.vertices.Load<Vertex>(vertexID);
	float4x4 modelMatrix = pushConstants.modelMatrices.Load<float4x4>(instanceIndex);

	VertexOutput output;
	output.position = mul(pushConstants.projection, mul(modelMatrix, float4(vertex.position, 1.0f)));
	output.color = vertex.color.xyz;
	output.uv = float2(vertex.uvX, vertex.uvY);
	return output;
}