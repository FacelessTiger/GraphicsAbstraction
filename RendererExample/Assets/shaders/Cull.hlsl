#include "Cobra"

struct Bounds
{
	float3 origin;
	float sphereRadius;
	float3 extents;
	uint transform;
};

struct CullMesh
{
	Bounds bounds;
	Cobra::DrawIndexedIndirectCommand command;
};

struct PushConstant
{
	row_major float4x4 viewProj;
	uint inputBuffer;
	uint inputCount;
	uint outputBuffer;
	uint models;
};
PushConstant(PushConstant, pushConstants);

bool IsVisible(CullMesh mesh)
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

	float4x4 modelMatrix = Cobra::ArrayBuffer::Create(pushConstants.models).Load<float4x4>(mesh.bounds.transform);
	float4x4 matrix = mul(modelMatrix, pushConstants.viewProj);

	for (int i = 0; i < 8; i++)
	{
		float4 v = mul(float4(mesh.bounds.origin + (corners[i] * mesh.bounds.extents), 1.0), matrix);
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

[numthreads(16, 1, 1)]
void main(uint3 dd: SV_DispatchThreadID)
{
	uint groupIndex = dd.x;
	Cobra::ArrayBuffer inputBuffer = Cobra::ArrayBuffer::Create(pushConstants.inputBuffer);
	Cobra::RWArrayBuffer outputBuffer = Cobra::RWArrayBuffer::Create(pushConstants.outputBuffer);

	if (groupIndex < pushConstants.inputCount)
	{
		CullMesh mesh = inputBuffer.Load<CullMesh>(groupIndex);
		if (IsVisible(mesh))
		{
			uint index = outputBuffer.InterlockedAdd(0, 1);
			outputBuffer.Store<Cobra::DrawIndexedIndirectCommand>(index, mesh.command, 4);
		}
	}
}