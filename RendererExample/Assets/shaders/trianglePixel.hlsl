struct VertexInput
{
	float3 color: COLOR0;
	float2 uv: TEXCOORD0;
};

float4 main(VertexInput input): SV_Target
{
	return float4(input.color, 1.0f);
}