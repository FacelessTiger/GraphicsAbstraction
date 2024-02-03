#include "Cobra"

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
	uint vertices;
	uint material;
	uint transform;
};

struct Material
{
	float3 albedoFactor;
	uint albedoTexture;
	uint metallicRoughnessTexture;
	float metallicFactor;
	float roughnessFactor;
	float ao;
};

struct PushConstant
{
	row_major float4x4 projection;
	float3 cameraPos;
	uint objects;
	uint materials;
	uint lights;
	uint lightCount;
	uint models;
	uint sampler;
};
PushConstant(PushConstant, pushConstants);

struct VertexOutput
{
	float4 position: SV_Position;
	float3 worldPosition: POSITION0;
	float3 normal: NORMAL0;
	Material material: COLOR0;
	float2 uv: TEXCOORD0;
};

VertexOutput main(uint vertexID: SV_VertexID, uint instanceIndex: SV_InstanceID)
{
	Object object = Cobra::ArrayBuffer::Create(pushConstants.objects).Load<Object>(instanceIndex);
	Vertex vertex = Cobra::ArrayBuffer::Create(object.vertices).Load<Vertex>(vertexID);
	Material material = Cobra::ArrayBuffer::Create(pushConstants.materials).Load<Material>(object.material);
	float4x4 modelMatrix = Cobra::ArrayBuffer::Create(pushConstants.models).Load<float4x4>(object.transform);

	float4 worldPos = mul(float4(vertex.position, 1.0f), modelMatrix);

	VertexOutput output;
	output.position = mul(worldPos, pushConstants.projection);
	output.worldPosition = worldPos.xyz;
	output.normal = mul(vertex.normal, (float3x3)transpose(Cobra::Inverse(modelMatrix)));
	output.material = material;
	output.uv = float2(vertex.uvX, vertex.uvY);
	return output;
}