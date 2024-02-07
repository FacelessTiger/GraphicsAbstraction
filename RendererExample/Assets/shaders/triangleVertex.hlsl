#include "Cobra"

struct Vertex
{
	float3 position;
	float uvX;
	float3 normal;
	float uvY;
	float4 color;
};

struct Primitive
{
	float3 origin;
	float sphereRadius;
	float3 extents;
	uint indexCount;
	uint indexOffset;
	uint materialOffset; // TODO: I feel like this should be tied to the draw data instead
	uint verticesOffset;
};

struct DrawData
{
	uint transformOffset;
	uint primitiveOffset;
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
	uint scene;
	uint drawsOffset;
	uint lightOffset;
	uint lightCount;
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
	Cobra::RawBuffer scene = Cobra::RawBuffer::Create(pushConstants.scene);
	DrawData draw = scene.Load<DrawData>((sizeof(DrawData) * instanceIndex) + pushConstants.drawsOffset);
	Primitive primitive = scene.Load<Primitive>(draw.primitiveOffset);
	float4x4 modelMatrix = scene.Load<float4x4>(draw.transformOffset);

	Vertex vertex = scene.Load<Vertex>((sizeof(Vertex) * vertexID) + primitive.verticesOffset);
	Material material = scene.Load<Material>(primitive.materialOffset);

	float4 worldPos = mul(float4(vertex.position, 1.0f), modelMatrix);

	VertexOutput output;
	output.position = mul(worldPos, pushConstants.projection);
	output.worldPosition = worldPos.xyz;
	output.normal = mul(vertex.normal, (float3x3)transpose(Cobra::Inverse(modelMatrix)));
	output.material = material;
	output.uv = float2(vertex.uvX, vertex.uvY);
	return output;
}