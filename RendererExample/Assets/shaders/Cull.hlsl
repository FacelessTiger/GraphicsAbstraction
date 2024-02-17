#include "Cobra"

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
	uint entityID;
};

struct PushConstant
{
	row_major float4x4 viewProj;
	uint sceneBuffer;
	uint inputCount;
	uint inputOffset;
	uint outputOffset;
};
PushConstant(PushConstant, pushConstants);

bool IsVisible(Primitive primitive, float4x4 transform)
{
	const float3 corners[8] = {
		float3( 1,  1,  1),
		float3( 1,  1, -1),
		float3( 1, -1,  1),
		float3( 1, -1, -1),
		float3(-1,  1,  1),
		float3(-1,  1, -1),
		float3(-1, -1,  1),
		float3(-1, -1, -1)
	};

	float3 vMin = float3(1.5, 1.5, 1.5);
	float3 vMax = float3(-1.5, -1.5, -1.5);

	float4x4 matrix = mul(transform, pushConstants.viewProj);
	for (int i = 0; i < 8; i++)
	{
		float4 v = mul(float4(primitive.origin + (corners[i] * primitive.extents), 1.0), matrix);
		v.x /= v.w;
		v.y /= v.w;
		v.z /= v.w;

		vMin = min((float3)v, vMin);
		vMax = max((float3)v, vMax);
	}

	if (vMin.z > 1.0 || vMax.z < 0.0 || vMin.x > 1.0 || vMax.x < -1.0 || vMin.y > 1.0 || vMax.y < -1.0)
		return false;
	else
		return true;
}

[numthreads(1024, 1, 1)]
void main(uint3 dd: SV_DispatchThreadID)
{
	uint groupIndex = dd.x;
	Cobra::RWRawBuffer sceneBuffer = Cobra::RWRawBuffer::Create(pushConstants.sceneBuffer);

	if (groupIndex < pushConstants.inputCount)
	{
		DrawData draw = sceneBuffer.Load<DrawData>((sizeof(DrawData) * groupIndex) + pushConstants.inputOffset);
		Primitive primitive = sceneBuffer.Load<Primitive>(draw.primitiveOffset);

		float4x4 transform = sceneBuffer.Load<float4x4>(draw.transformOffset);
		if (IsVisible(primitive, transform))
		{
			uint index = sceneBuffer.InterlockedAdd(pushConstants.outputOffset, 1);

			Cobra::DrawIndexedIndirectCommand command = Cobra::DrawIndexedIndirectCommand::Create(primitive.indexCount, 1, primitive.indexOffset, 0, groupIndex);
			sceneBuffer.Store<Cobra::DrawIndexedIndirectCommand>((index * sizeof(Cobra::DrawIndexedIndirectCommand)) + pushConstants.outputOffset + 4, command);
		}
	}
}