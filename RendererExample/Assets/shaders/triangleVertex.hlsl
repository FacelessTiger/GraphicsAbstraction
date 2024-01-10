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
	uint vertices;
	uint modelMatrices;
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
	Vertex vertex = ArrayBuffer::Create(pushConstants.vertices).Load<Vertex>(vertexID);
	float4x4 modelMatrix = ArrayBuffer::Create(pushConstants.modelMatrices).Load<float4x4>(instanceIndex);

	VertexOutput output;
	// TODO: for some reason this matrix multiplication is borked, even if I changed the row/column major
	#ifdef VK_BINDLESS
	output.position = mul(pushConstants.projection, mul(modelMatrix, float4(vertex.position, 1.0f)));
	#else
	output.position = mul(pushConstants.projection, mul(float4(vertex.position, 1.0f), modelMatrix));
	#endif
	output.color = vertex.color.xyz;
	output.uv = float2(vertex.uvX, vertex.uvY);
	return output;
}