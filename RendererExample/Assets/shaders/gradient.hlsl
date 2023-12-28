#include "bindless.hlsl"

struct Data
{
	float4 data1;
	float4 data2;
};

struct PushConstant
{
	RwTexture image;
	SimpleBuffer buffer;
};
PushConstant(PushConstant, pushConstants);

[numthreads(16, 16, 1)]
void main(uint3 threadID: SV_DispatchThreadID, uint3 groupID: SV_GroupThreadID)
{
	Data data = pushConstants.buffer.Load<Data>();

	int2 texelCoord = int2(threadID.xy);
	int2 size = pushConstants.image.GetDimensions<float4>();

	float4 topColor = data.data1;
	float4 bottomColor = data.data2;

	if (texelCoord.x < size.x && texelCoord.y < size.y)
	{
		float blend = float(texelCoord.y)/size.y;
		pushConstants.image.store2D(texelCoord, lerp(topColor, bottomColor, blend));
	}
}