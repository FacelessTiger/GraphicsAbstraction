#include "bindless.hlsl"

struct Data
{
	float4 data1;
	float4 data2;
};

struct Bindings
{
	RwTexture image;
	SimpleBuffer buffer;
};

[numthreads(16, 16, 1)]
void main(uint3 threadID: SV_DispatchThreadID, uint3 groupID: SV_GroupThreadID)
{
	Bindings bnd = loadBindings<Bindings>();
	Data data = bnd.buffer.Load<Data>();

	int2 texelCoord = int2(threadID.xy);
	int2 size = bnd.image.GetDimensions<float4>();

	float4 topColor = data.data1;
	float4 bottomColor = data.data2;

	if (texelCoord.x < size.x && texelCoord.y < size.y)
	{
		float blend = float(texelCoord.y)/size.y;
		bnd.image.store2D(texelCoord, lerp(topColor, bottomColor, blend));
	}
}