struct RenderResourceHandle
{
	uint index;
};

#include "descriptorHeap.hlsl"

struct ArrayBuffer
{
	RenderResourceHandle handle;

	template <typename ReadStructure>
	ReadStructure Load(uint index, uint size = 0)
	{
		uint byteSize = size ? size : sizeof(ReadStructure);
		ByteAddressBuffer buffer = DESCRIPTOR_HEAP(ByteBufferHandle, this.handle.index);
		ReadStructure result = buffer.Load<ReadStructure>(byteSize * index);

		return result;
	}
};

struct SimpleBuffer
{
	RenderResourceHandle handle;

	template <typename ReadStructure>
	ReadStructure Load()
	{
		ByteAddressBuffer buffer = DESCRIPTOR_HEAP(ByteBufferHandle, this.handle.index);
		ReadStructure result = buffer.Load<ReadStructure>(0);

		return result;
	}
};

struct Sampler
{
	RenderResourceHandle handle;
};

struct Texture
{
	RenderResourceHandle handle;

	template <typename TextureValue>
	TextureValue Sample2D(Sampler s, float2 uv)
	{
		Texture2D<TextureValue> texture = DESCRIPTOR_HEAP(Texture2DHandle<TextureValue>, this.handle.index);
		SamplerState sampler = DESCRIPTOR_HEAP(SamplerHandle, s.handle.index);

		return texture.Sample(sampler, uv);
	}
};

struct RwTexture
{
	RenderResourceHandle handle;

	template <typename RWTextureValue>
	int2 GetDimensions()
	{
		RWTexture2D<RWTextureValue> texture = DESCRIPTOR_HEAP(RWTexture2DHandle<RWTextureValue>, this.handle.index);

		int2 size;
		texture.GetDimensions(size.x, size.y);
		return size;
	}

	template <typename RWTextureValue>
	void store2D(uint2 pos, RWTextureValue value)
	{
		RWTexture2D<RWTextureValue> texture = DESCRIPTOR_HEAP(RWTexture2DHandle<RWTextureValue>, this.handle.index);
		texture[pos] = value;
	}
};

float4 UnpackUnorm4x8(uint value)
{
    uint4 Packed = uint4(value & 0xff, (value >> 8) & 0xff, (value >> 16) & 0xff, value >> 24);
    return float4(Packed) / 255.0;
}

struct PushConstantStruct
{
#if __SHADER_TARGET_STAGE == __SHADER_STAGE_VERTEX
	[[vk::offset(0)]]
#elif __SHADER_TARGET_STAGE == __SHADER_STAGE_PIXEL
	[[vk::offset(4)]]
#elif __SHADER_TARGET_STAGE == __SHADER_STAGE_COMPUTE
	[[vk::offset(8)]]
#endif
	RenderResourceHandle handle;
#ifdef PushConstants
	PushConstants
#endif
};

[[vk::push_constant]] ConstantBuffer<PushConstantStruct> g_PushConstants;

template <typename T>
T loadBindings()
{
	ByteAddressBuffer b = g_ByteAddressBuffer[g_PushConstants.handle.index];
	T result = b.Load<T>(0);
	return result;
}