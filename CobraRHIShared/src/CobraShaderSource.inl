#pragma once

static const char* s_DescriptorHeapSource = R"(
namespace Cobra {

    #ifdef VK_BINDLESS
        #define DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(textureType, bindingA, bindingB)                    \
            [[vk::binding(bindingA, bindingB)]] textureType<float>                                         \
                g##_##textureType##float[];                                                                \
            [[vk::binding(bindingA, bindingB)]] textureType<float2>                                        \
                g##_##textureType##float2[];                                                               \
            [[vk::binding(bindingA, bindingB)]] textureType<float3>                                        \
                g##_##textureType##float3[];                                                               \
            [[vk::binding(bindingA, bindingB)]] textureType<float4>                                        \
                g##_##textureType##float4[];

        DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(Texture1D, 3, 0)
        DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(Texture2D, 3, 0)
        DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(Texture3D, 3, 0)
        DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(RWTexture1D, 2, 0)
        DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(RWTexture2D, 2, 0)
        DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(RWTexture3D, 2, 0)

        [[vk::binding(0, 0)]] ByteAddressBuffer g_ByteAddressBuffer[];
        [[vk::binding(0, 0)]] RWByteAddressBuffer g_RWByteAddressBuffer[];
        [[vk::binding(1, 0)]] SamplerState g_SamplerState[];

        struct ByteBufferHandle { uint internalIndex; };
        struct RWByteBufferHandle { uint internalIndex; };

        template <typename T> struct Texture2DHandle { uint internalIndex; };
        template <typename T> struct RWTexture2DHandle { uint internalIndex; };

        #define TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, registerName, valueType,           \
                                                          handleName)                                      \
            resourceType<valueType> operator[](handleName<valueType> identifier)						   \
            {                         																	   \
                return registerName##valueType[NonUniformResourceIndex(identifier.internalIndex)];         \
            }

        #define TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(resourceType)                                          \
            TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, g##_##resourceType, float, resourceType##Handle)   \
            TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, g##_##resourceType, float2, resourceType##Handle)  \
            TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, g##_##resourceType, float3, resourceType##Handle)  \
            TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, g##_##resourceType, float4, resourceType##Handle)    

        struct VulkanResourceDescriptorHeapInternal
        {
        	ByteAddressBuffer operator[](ByteBufferHandle identifier) { return g_ByteAddressBuffer[NonUniformResourceIndex(identifier.internalIndex)]; }
            RWByteAddressBuffer operator[](RWByteBufferHandle identifier) { return g_RWByteAddressBuffer[NonUniformResourceIndex(identifier.internalIndex)]; }

        	TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(Texture2D)
            TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(RWTexture2D)
        };

        static VulkanResourceDescriptorHeapInternal VkResourceDescriptorHeap;
        #define DESCRIPTOR_HEAP(handleType, handle) VkResourceDescriptorHeap[(handleType)handle]
        #define SAMPLER_HEAP(handle) g_SamplerState[NonUniformResourceIndex(handle)]
    #else
        #define DESCRIPTOR_HEAP(handleType, handle) ResourceDescriptorHeap[NonUniformResourceIndex(handle)]
        #define SAMPLER_HEAP(handle) SamplerDescriptorHeap[NonUniformResourceIndex(handle)]
    #endif

}
)";

static const char* s_BindlessSource = R"(
#include "DescriptorHeap"

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

	struct RawBuffer
	{
		RenderResourceHandle handle;

		static RawBuffer Create(uint index)
		{
			RawBuffer ret;
			ret.handle.index = index;

			return ret;
		}

		template <typename T>
		T Load(uint index)
		{
			ByteAddressBuffer buffer = DESCRIPTOR_HEAP(ByteBufferHandle, this.handle.GetReadIndex());
			T result = buffer.Load<T>(index);

			return result;
		}
	};

	struct RWRawBuffer
	{
		RenderResourceHandle handle;

		static RWRawBuffer Create(uint index)
		{
			RWRawBuffer ret;
			ret.handle.index = index;

			return ret;
		}

		template <typename T>
		T Load(uint index)
		{
			ByteAddressBuffer buffer = DESCRIPTOR_HEAP(ByteBufferHandle, this.handle.GetReadIndex());
			T result = buffer.Load<T>(index);

			return result;
		}

		template <typename T>
		void Store(uint index, T value)
		{
			RWByteAddressBuffer buffer = DESCRIPTOR_HEAP(RWByteBufferHandle, this.handle.GetWriteIndex());
			buffer.Store<T>(index, value);
		}

		uint InterlockedAdd(uint dest, uint value)
		{
			RWByteAddressBuffer buffer = DESCRIPTOR_HEAP(RWByteBufferHandle, this.handle.GetWriteIndex());

			uint originalValue;
			buffer.InterlockedAdd(dest, value, originalValue);
			return originalValue;
		}
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

		template <typename T>
		T Load(uint index)
		{
			ByteAddressBuffer buffer = DESCRIPTOR_HEAP(ByteBufferHandle, this.handle.GetReadIndex());
			T result = buffer.Load<T>(index * sizeof(T));

			return result;
		}

		template <typename T>
		uint GetDimensions()
		{
			ByteAddressBuffer buffer = DESCRIPTOR_HEAP(ByteBufferHandle, this.handle.GetReadIndex());

			uint dimension;
			buffer.GetDimensions(dimension);
			return dimension / sizeof(T);
		}
	};

	struct RWArrayBuffer
	{
		RenderResourceHandle handle;

		static RWArrayBuffer Create(uint index)
		{
			RWArrayBuffer ret;
			ret.handle.index = index;

			return ret;
		}

		template <typename T>
		T Load(uint index)
		{
			ByteAddressBuffer buffer = DESCRIPTOR_HEAP(ByteBufferHandle, this.handle.GetReadIndex());
			T result = buffer.Load<T>(index * sizeof(T));

			return result;
		}

		template <typename T>
		void Store(uint index, T value, uint offset = 0)
		{
			RWByteAddressBuffer buffer = DESCRIPTOR_HEAP(RWByteBufferHandle, this.handle.GetWriteIndex());
			buffer.Store<T>(index * sizeof(T) + offset, value);
		}

		uint InterlockedAdd(uint dest, uint value)
		{
			RWByteAddressBuffer buffer = DESCRIPTOR_HEAP(RWByteBufferHandle, this.handle.GetWriteIndex());

			uint originalValue;
			buffer.InterlockedAdd(dest, value, originalValue);
			return originalValue;
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
)";

static const char* s_UtilsSource = R"(
namespace Cobra {

	struct DrawIndexedIndirectCommand
	{
		uint reserved, reserved2;
		uint indexCount;
		uint instanceCount;
		uint firstIndex;
		uint vertexOffset;
		uint firstInstance;

		static DrawIndexedIndirectCommand Create(uint indexCount, uint instanceCount, uint firstIndex, uint vertexOffset, uint firstInstance)
		{
			DrawIndexedIndirectCommand ret;
			ret.indexCount = indexCount;
			ret.instanceCount = instanceCount;
			ret.firstIndex = firstIndex;
			ret.vertexOffset = vertexOffset;
			ret.firstInstance = firstInstance;
			ret.reserved = vertexOffset;
			ret.reserved2 = firstInstance;

			return ret;
		};
	};

	float4 UnpackUnorm4x8(uint value)
	{
	    uint4 Packed = uint4(value & 0xff, (value >> 8) & 0xff, (value >> 16) & 0xff, value >> 24);
	    return float4(Packed) / 255.0;
	}

	float4x4 Inverse(float4x4 m)
	{
	    float n11 = m[0][0], n12 = m[1][0], n13 = m[2][0], n14 = m[3][0];
	    float n21 = m[0][1], n22 = m[1][1], n23 = m[2][1], n24 = m[3][1];
	    float n31 = m[0][2], n32 = m[1][2], n33 = m[2][2], n34 = m[3][2];
	    float n41 = m[0][3], n42 = m[1][3], n43 = m[2][3], n44 = m[3][3];

	    float t11 = n23 * n34 * n42 - n24 * n33 * n42 + n24 * n32 * n43 - n22 * n34 * n43 - n23 * n32 * n44 + n22 * n33 * n44;
	    float t12 = n14 * n33 * n42 - n13 * n34 * n42 - n14 * n32 * n43 + n12 * n34 * n43 + n13 * n32 * n44 - n12 * n33 * n44;
	    float t13 = n13 * n24 * n42 - n14 * n23 * n42 + n14 * n22 * n43 - n12 * n24 * n43 - n13 * n22 * n44 + n12 * n23 * n44;
	    float t14 = n14 * n23 * n32 - n13 * n24 * n32 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34;

	    float det = n11 * t11 + n21 * t12 + n31 * t13 + n41 * t14;
	    float idet = 1.0f / det;

	    float4x4 ret;

	    ret[0][0] = t11 * idet;
	    ret[0][1] = (n24 * n33 * n41 - n23 * n34 * n41 - n24 * n31 * n43 + n21 * n34 * n43 + n23 * n31 * n44 - n21 * n33 * n44) * idet;
	    ret[0][2] = (n22 * n34 * n41 - n24 * n32 * n41 + n24 * n31 * n42 - n21 * n34 * n42 - n22 * n31 * n44 + n21 * n32 * n44) * idet;
	    ret[0][3] = (n23 * n32 * n41 - n22 * n33 * n41 - n23 * n31 * n42 + n21 * n33 * n42 + n22 * n31 * n43 - n21 * n32 * n43) * idet;

	    ret[1][0] = t12 * idet;
	    ret[1][1] = (n13 * n34 * n41 - n14 * n33 * n41 + n14 * n31 * n43 - n11 * n34 * n43 - n13 * n31 * n44 + n11 * n33 * n44) * idet;
	    ret[1][2] = (n14 * n32 * n41 - n12 * n34 * n41 - n14 * n31 * n42 + n11 * n34 * n42 + n12 * n31 * n44 - n11 * n32 * n44) * idet;
	    ret[1][3] = (n12 * n33 * n41 - n13 * n32 * n41 + n13 * n31 * n42 - n11 * n33 * n42 - n12 * n31 * n43 + n11 * n32 * n43) * idet;

	    ret[2][0] = t13 * idet;
	    ret[2][1] = (n14 * n23 * n41 - n13 * n24 * n41 - n14 * n21 * n43 + n11 * n24 * n43 + n13 * n21 * n44 - n11 * n23 * n44) * idet;
	    ret[2][2] = (n12 * n24 * n41 - n14 * n22 * n41 + n14 * n21 * n42 - n11 * n24 * n42 - n12 * n21 * n44 + n11 * n22 * n44) * idet;
	    ret[2][3] = (n13 * n22 * n41 - n12 * n23 * n41 - n13 * n21 * n42 + n11 * n23 * n42 + n12 * n21 * n43 - n11 * n22 * n43) * idet;

	    ret[3][0] = t14 * idet;
	    ret[3][1] = (n13 * n24 * n31 - n14 * n23 * n31 + n14 * n21 * n33 - n11 * n24 * n33 - n13 * n21 * n34 + n11 * n23 * n34) * idet;
	    ret[3][2] = (n14 * n22 * n31 - n12 * n24 * n31 - n14 * n21 * n32 + n11 * n24 * n32 + n12 * n21 * n34 - n11 * n22 * n34) * idet;
	    ret[3][3] = (n12 * n23 * n31 - n13 * n22 * n31 + n13 * n21 * n32 - n11 * n23 * n32 - n12 * n21 * n33 + n11 * n22 * n33) * idet;

	    return ret;
	}

}
)";

static const char* s_CobraSource = R"(
#include "Bindless"
#include "Utils"
)";