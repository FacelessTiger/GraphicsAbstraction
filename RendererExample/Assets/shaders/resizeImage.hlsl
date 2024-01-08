#include "bindless.hlsl"

struct PushConstant
{
	uint srcImage;
	uint dstImage;
	uint sampler;
};
PushConstant(PushConstant, pushConstants);

[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID)
{
	Texture srcImage = Texture::Create(pushConstants.srcImage);
	RwTexture dstImage = RwTexture::Create(pushConstants.dstImage);

	float2 scaledUV = float2(dispatchThreadID.xy) / dstImage.GetDimensions<float4>();
	dstImage.Store2D(dispatchThreadID.xy, srcImage.SampleLevel2D<float4>(pushConstants.sampler, scaledUV, 0));
}