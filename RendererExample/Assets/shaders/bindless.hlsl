#include "DescriptorHeap.hlsl"

#ifdef VK_BINDLESS
	#define PushConstant(structName, variableName) [[vk::push_constant]] ConstantBuffer<structName> variableName
#else
	#define PushConstant(structName, variableName) ConstantBuffer<structName> variableName: register(b0, space0);

	struct SpecialConstant
	{
		uint vertexOffset;
		uint instanceOffset;
	};
	ConstantBuffer<SpecialConstant> g_SpecialConstants: register(b1, space0);
#endif

namespace Cobra {

	struct RenderResourceHandle
	{
		uint index;

		uint GetReadIndex() { return this.index; }
	#ifdef VK_BINDLESS
		uint GetWriteIndex() { return this.index; }
	#else
		uint GetWriteIndex() { return this.index + 1; }
	#endif
	};

	struct ArrayBuffer
	{
		RenderResourceHandle handle;

		static ArrayBuffer Create(uint index)
		{
			ArrayBuffer ret;
			ret.handle.index = index;

			return ret;
		}

		template <typename ReadStructure>
		ReadStructure Load(uint index)
		{
			ByteAddressBuffer buffer = DESCRIPTOR_HEAP(ByteBufferHandle, this.handle.GetReadIndex());
			ReadStructure result = buffer.Load<ReadStructure>(index * sizeof(ReadStructure));

			return result;
		}
	};


	struct SimpleBuffer
	{
		RenderResourceHandle handle;

		static SimpleBuffer Create(uint index)
		{
			SimpleBuffer ret;
			ret.handle.index = index;

			return ret;
		}

		template <typename ReadStructure>
		ReadStructure Load()
		{
			ByteAddressBuffer buffer = DESCRIPTOR_HEAP(ByteBufferHandle, this.handle.GetReadIndex());
			ReadStructure result = buffer.Load<ReadStructure>(0);

			return result;
		}
	};

	struct Texture
	{
		RenderResourceHandle handle;

		static Texture Create(uint index)
		{
			Texture ret;
			ret.handle.index = index;

			return ret;
		}

		template <typename TextureValue>
		int2 GetDimensions()
		{
			Texture2D<TextureValue> texture = DESCRIPTOR_HEAP(Texture2DHandle<TextureValue>, this.handle.GetReadIndex());

			int2 size;
			texture.GetDimensions(size.x, size.y);
			return size;
		}

		template <typename TextureValue>
		TextureValue Sample2D(uint s, float2 uv)
		{
			Texture2D<TextureValue> texture = DESCRIPTOR_HEAP(Texture2DHandle<TextureValue>, this.handle.GetReadIndex());
			SamplerState sampler = SAMPLER_HEAP(s);

			return texture.Sample(sampler, uv);
		}

		template <typename TextureValue>
		TextureValue SampleLevel2D(uint s, float2 uv, uint lod)
		{
			Texture2D<TextureValue> texture = DESCRIPTOR_HEAP(Texture2DHandle<TextureValue>, this.handle.GetReadIndex());
			SamplerState sampler = SAMPLER_HEAP(s);

			return texture.SampleLevel(sampler, uv, lod);
		}
	};

	struct RwTexture
	{
		RenderResourceHandle handle;

		static RwTexture Create(uint index)
		{
			RwTexture ret;
			ret.handle.index = index;
			
			return ret;
		}

		template <typename RWTextureValue>
		int2 GetDimensions()
		{
			RWTexture2D<RWTextureValue> texture = DESCRIPTOR_HEAP(RWTexture2DHandle<RWTextureValue>, this.handle.GetWriteIndex());

			int2 size;
			texture.GetDimensions(size.x, size.y);
			return size;
		}

		template <typename RWTextureValue>
		void Store2D(uint2 pos, RWTextureValue value)
		{
			RWTexture2D<RWTextureValue> texture = DESCRIPTOR_HEAP(RWTexture2DHandle<RWTextureValue>, this.handle.GetWriteIndex());
			texture[pos] = value;
		}
	};

}