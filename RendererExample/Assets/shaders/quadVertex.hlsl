#define PushConstants float4x4 projection;
#include "bindless.hlsl"

struct QuadData
{
	float2 scale;
	Texture texture;
	uint color;
	float3 position;
	float rotation;
};

struct Bindings
{
	ArrayBuffer quadData;
};

struct VertexOutput
{
	float4 position: SV_Position;
	float4 color: COLOR0;
	Texture texture: BLENDINDICES0;
	float2 uv: TEXCOORD0;
};

VertexOutput main(uint vertexID: SV_VertexID)
{
	const float2 vertices[6] = {
		float2( 0.5f, -0.5f),
		float2( 0.5f,  0.5f),
		float2(-0.5f, -0.5f),
		float2(-0.5f, -0.5f),
		float2( 0.5f,  0.5f),
		float2(-0.5f,  0.5f)
	};

	const float2 uvs[6] = {
		float2(1, 0),
		float2(1, 1),
		float2(0, 0),
		float2(0, 0),
		float2(1, 1),
		float2(0, 1)
	};

	Bindings bnd = loadBindings<Bindings>();
	int quadID = vertexID / 6;
	int relativeVertexID = vertexID % 6;

	QuadData quad = bnd.quadData.Load<QuadData>(quadID);
	float2 pos = vertices[relativeVertexID];

	float2 cossin = float2(cos(quad.rotation), sin(quad.rotation));
	float2 rotatedPos = float2(pos.x * cossin.x - pos.y * cossin.y, pos.x * cossin.y + pos.y * cossin.x);
	float2 relativePos = rotatedPos * quad.scale + quad.position.xy;

	VertexOutput output;
	output.position = mul(g_PushConstants.projection, float4(relativePos, quad.position.z, 1.0f));
	output.color = UnpackUnorm4x8(quad.color);
	output.texture = quad.texture;
	output.uv = uvs[relativeVertexID];
	return output;
}