#include "Cobra.hlsl"

struct Data
{
	float4 data1;
	float4 data2;
};

struct PushConstant
{
	uint image;
	uint buffer;
};
PushConstant(PushConstant, pushConstants);

[numthreads(16, 16, 1)]
void main(uint3 threadID: SV_DispatchThreadID, uint3 groupID: SV_GroupThreadID)
{
	Cobra::RwTexture image = Cobra::RwTexture::Create(pushConstants.image);
	Data data = Cobra::SimpleBuffer::Create(pushConstants.buffer).Load<Data>();

	int2 texelCoord = int2(threadID.xy);
	int2 size = image.GetDimensions<float4>();

	float4 topColor = data.data1;
	float4 bottomColor = data.data2;

	if (texelCoord.x < size.x && texelCoord.y < size.y)
	{
		float blend = float(texelCoord.y)/size.y;
		image.Store2D(texelCoord, lerp(topColor, bottomColor, blend));
	}
}