#      :              �    +     ,     1     �    �  
  SPV_EXT_descriptor_indexing                  main                      
    Assets/shaders/imguiPixel.hlsl   s    �     #define PushConstants	[[vk::offset(16)]] float2 scale; \
						[[vk::offset(24)]] float2 offset; \
						[[vk::offset(32)]] Texture texture; \
						[[vk::offset(36)]] Sampler sampler;
#include "bindless.hlsl"

struct VertexInput
{
	float4 color: COLOR0;
	float2 uv: TEXCOORD0;
};

float4 main(VertexInput input): SV_Target
{
	return input.color * g_PushConstants.texture.Sample2D<float4>(g_PushConstants.sampler, input.uv);
}    
    Assets/shaders/bindless.hlsl     d   �     struct RenderResourceHandle
{
	uint index;
};

#include "descriptorHeap.hlsl"

struct ArrayBuffer
{
	RenderResourceHandle handle;

	template <typename ReadStructure>
	ReadStructure Load(uint index)
	{
		ByteAddressBuffer buffer = DESCRIPTOR_HEAP(ByteBufferHandle, this.handle.index);
		ReadStructure result = buffer.Load<ReadStructure>(sizeof(ReadStructure) * index);

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
}      Assets/shaders/descriptorHeap.hlsl   )   �     #define DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(textureType, bindingA, bindingB)                    \
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
[[vk::binding(1, 0)]] SamplerState g_SamplerState[];

struct ByteBufferHandle { uint internalIndex; };
struct SamplerHandle { uint internalIndex; };

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
    SamplerState operator[](SamplerHandle identifier) { return g_SamplerState[NonUniformResourceIndex(identifier.internalIndex)]; }

	TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(Texture2D)
    TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(RWTexture2D)
};

static VulkanResourceDescriptorHeapInternal VkResourceDescriptorHeap;

#define DESCRIPTOR_HEAP(handleType, handle) VkResourceDescriptorHeap[(handleType)handle]         type.2d.image     	   g_Texture2Dfloat4     
   type.sampler         g_SamplerState       type.PushConstant.PushConstantStruct             handle          scale           offset          texture         sampler      RenderResourceHandle             index        Texture          handle       Sampler          handle       g_PushConstants      in.var.COLOR0        in.var.TEXCOORD0         out.var.SV_Target        main         type.sampled.image  G            G           G            G  	   "       G  	   !      G     "       G     !      H         #       H         #       H         #       H         #      H        #      H        #      H        #       H        #   $   G        G     �  G     �  G     �  G     �  G     �  G     �              +            +           +                     	                                                                
         
      !                     "                                 "   "            #   	        $            %      $      &      "      '      $     (   !  )   (      *   	                 +             ,       
   ;     	       ;  !          ;  #      	   ;  %         ;  &         ;  '            -   	      6  (          )   �  .   =  $   /      =  "   0              @   A  *   1                 P   =     2   1           @   Q     3   2       Q     4   3            0   ]   A  -   5                    0   i   =     6   5        -     S        6        -   �  A  +      	      =                +   O   S        4        +   @   A  ,            =  
              3   
   V     7         W  $   8   7   0                  �  $   9   /   8   = >     9              �  8  