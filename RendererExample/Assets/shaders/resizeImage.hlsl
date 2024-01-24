#include "Cobra"

struct PushConstant
{
	uint srcImage;
	uint dstImage;
	uint sampler;
};
PushConstant(PushConstant, pushConstants);

[numthreads(32, 32, 1)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID)
{
	Cobra::Texture srcImage = Cobra::Texture::Create(pushConstants.srcImage);
	Cobra::RwTexture dstImage = Cobra::RwTexture::Create(pushConstants.dstImage);

	float2 scaledUV = float2(dispatchThreadID.xy) / dstImage.GetDimensions<float4>();
	dstImage.Store2D(dispatchThreadID.xy, srcImage.SampleLevel2D<float4>(pushConstants.sampler, scaledUV, 0));
}