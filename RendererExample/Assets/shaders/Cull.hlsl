#include "Cobra.hlsl"

struct CullMesh
{
	Cobra::DrawIndexedIndirectCommand command;
};

struct PushConstant
{
	uint inputBuffer;
	uint outputBuffer;
};
PushConstant(PushConstant, pushConstants);

[numthreads(16, 1, 1)]
void main(uint groupIndex: SV_GroupIndex)
{
	Cobra::ArrayBuffer inputBuffer = Cobra::ArrayBuffer::Create(pushConstants.inputBuffer);
	Cobra::RWArrayBuffer outputBuffer = Cobra::RWArrayBuffer::Create(pushConstants.outputBuffer);

	if (groupIndex < inputBuffer.GetDimensions<CullMesh>())
	{
		CullMesh mesh = inputBuffer.Load<CullMesh>(groupIndex);
		outputBuffer.Store<CullMesh>(groupIndex, mesh);
	}
}