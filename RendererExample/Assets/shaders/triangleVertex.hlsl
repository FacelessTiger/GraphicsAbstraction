#include "Cobra.hlsl"

struct Vertex
{
	float3 position;
	float uvX;
	float3 normal;
	float uvY;
	float4 color;
};

struct Object
{
	float4x4 modelMatrix;
	uint vertices;
};

struct PushConstant
{
	float4x4 projection;
	float3 cameraPos;
	uint objects;
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
	Object object = Cobra::ArrayBuffer::Create(pushConstants.objects).Load<Object>(instanceIndex);
	Vertex vertex = Cobra::ArrayBuffer::Create(object.vertices).Load<Vertex>(vertexID);

	float4 worldPos = mul(float4(vertex.position, 1.0f), object.modelMatrix);

	VertexOutput output;
	output.position = mul(pushConstants.projection, worldPos);
	output.worldPosition = worldPos.xyz;
	output.normal = mul(vertex.normal, (float3x3)transpose(Cobra::Inverse(object.modelMatrix)));
	output.color = vertex.color.xyz;
	output.uv = float2(vertex.uvX, vertex.uvY);
	return output;
}