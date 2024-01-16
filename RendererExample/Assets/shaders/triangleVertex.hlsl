#include "Cobra.hlsl"

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
	float3 cameraPos;
	uint vertices;
	uint modelMatrices;
};
PushConstant(PushConstant, pushConstants);

struct VertexOutput
{
	float4 position: SV_Position;
	float3 worldPosition: POSITION0;
	float3 normal: NORMAL0;
	float3 color: COLOR0;
	float2 uv: TEXCOORD0;
};

VertexOutput main(uint vertexID: SV_VertexID, uint instanceIndex: SV_InstanceID)
{
	Vertex vertex = Cobra::ArrayBuffer::Create(pushConstants.vertices).Load<Vertex>(vertexID);
	float4x4 modelMatrix = Cobra::ArrayBuffer::Create(pushConstants.modelMatrices).Load<float4x4>(instanceIndex);

	float4 worldPos = mul(float4(vertex.position, 1.0f), modelMatrix);

	VertexOutput output;
	output.position = mul(pushConstants.projection, worldPos);
	output.worldPosition = worldPos.xyz;
	output.normal = mul(vertex.normal, (float3x3)transpose(Cobra::Inverse(modelMatrix)));
	output.color = vertex.color.xyz;
	output.uv = float2(vertex.uvX, vertex.uvY);
	return output;
}