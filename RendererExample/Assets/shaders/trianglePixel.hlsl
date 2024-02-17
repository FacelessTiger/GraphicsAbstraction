#include "Cobra"

static const float PI = 3.14159265f;

struct Material
{
	float4 albedoFactor;
	uint albedoTexture;
	uint metallicRoughnessTexture;
	float metallicFactor;
	float roughnessFactor;
	float ao;
};

struct Light
{
	float3 position;
	float padding;
	float3 color;
	float padding2;
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

struct VertexInput
{
	float4 position: SV_Position;
	float3 worldPosition: POSITION0;
	float3 normal: NORMAL0;
	Material material: COLOR0;
	uint entityID: POSITION1;
	float2 uv: TEXCOORD0;
};

struct PixelOutput
{
	float4 color: SV_Target0;
	int entityID: SV_Target1;
};

float4 FresnelShlick(float cosTheta, float4 f0)
{
	return f0 + (1.0 - f0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(float3 n, float3 h, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float nDotH = max(dot(n, h), 0.0);
	float nDotH2 = nDotH * nDotH;

	float num = a2;
	float denom = (nDotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / denom;
}

float GeometrySchlickGGX(float nDotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float num = nDotV;
	float denom = nDotV * (1.0 - k) + k;

	return num / denom;
}

float GeometrySmith(float3 n, float3 v, float3 l, float roughness)
{
	float nDotV = max(dot(n, v), 0.0);
	float nDotL = max(dot(n, l), 0.0);
	float ggx2 = GeometrySchlickGGX(nDotV, roughness);
	float ggx1 = GeometrySchlickGGX(nDotL, roughness);

	return ggx1 * ggx2;
}

PixelOutput main(VertexInput input)
{
	Cobra::RawBuffer scene = Cobra::RawBuffer::Create(pushConstants.scene);
	float4 metallicRoughnessSample = Cobra::Texture::Create(input.material.metallicRoughnessTexture).Sample2D<float4>(pushConstants.sampler, input.uv);

	float4 albedo = Cobra::Texture::Create(input.material.albedoTexture).Sample2D<float4>(pushConstants.sampler, input.uv) * input.material.albedoFactor;
	float metallic = metallicRoughnessSample.b * input.material.metallicFactor;
	float roughness = metallicRoughnessSample.g * input.material.roughnessFactor;
	float ao = input.material.ao;

	float3 n = normalize(input.normal);
	float3 v = normalize(pushConstants.cameraPos - input.worldPosition);

	float4 f0 = 0.04;
	f0 = lerp(f0, albedo, metallic);

	float4 lo = 0.0;
	for (int i = 0; i < pushConstants.lightCount; i++)
	{
		Light light = scene.Load<Light>((i * sizeof(Light)) + pushConstants.lightOffset);

		float3 l = normalize(light.position - input.worldPosition);
		float3 h = normalize(v + l);

		float distance = length(light.position - input.worldPosition);
		float attenuation = 1.0 / (distance * distance);
		float3 radiance = light.color * attenuation;

		float ndf = DistributionGGX(n, h, roughness);
		float g = GeometrySmith(n, v, l, roughness);
		float4 f = FresnelShlick(max(dot(h, v), 0.0), f0);

		float4 kS = f;
		float4 kD = 1.0 - kS;
		kD *= 1.0 - metallic;

		float4 numerator = ndf * g * f;
		float denominator = 4.0 * max(dot(n, v), 0.0) * max(dot(n, l), 0.0) + 0.0001;
		float4 specular = numerator / denominator;

		float nDotL = max(dot(n, l), 0.0);
		lo += (kD * albedo / PI + specular) * float4(radiance, 1.0) * nDotL;
	}

	float4 ambient = 0.03 * albedo * ao;
	float4 color = ambient + lo;

	color = color / (color + 1.0);
	color = pow(color, 1.0 / 2.2);
	color.w = albedo.w;

	PixelOutput output;
	output.color = color;
	output.entityID = input.entityID;
	return output;
}