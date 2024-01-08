#      \              �    +     ,     1     �  
  SPV_EXT_descriptor_indexing           	        main                     Assets/shaders/triangleVertex.hlsl   �    �     #include "bindless.hlsl"

struct Vertex
{
	float3 position;
	float uvX;
	float3 normal;
	float uvY;
	float4 color;
};

struct PushConstant
{
	float4x4 projection;
	uint vertices;
};
PushConstant(PushConstant, pushConstants);

struct VertexOutput
{
	float4 position: SV_Position;
	float3 color: COLOR0;
	float2 uv: TEXCOORD0;
};

VertexOutput main(uint vertexID: SV_VertexID)
{
	Vertex vertex = ArrayBuffer::Create(pushConstants.vertices).Load<Vertex>(vertexID);

	VertexOutput output;
	output.position = mul(pushConstants.projection, float4(vertex.position, 1.0f));
	output.color = vertex.color.xyz;
	output.uv = float2(vertex.uvX, vertex.uvY);
	return output;
}     
    Assets/shaders/bindless.hlsl     B   �     struct RenderResourceHandle
{
	uint index;

	uint GetReadIndex() { return this.index; }
#ifdef VK_BINDLESS
	uint GetWriteIndex() { return this.index; }
#else
	uint GetWriteIndex() { return this.index + 1; }
#endif
};

#include "descriptorHeap.hlsl"

#ifdef VK_BINDLESS
	#define PushConstant(structName, variableName) [[vk::push_constant]] ConstantBuffer<structName> variableName
#else
	#define PushConstant(structName, variableName) ConstantBuffer<structName> variableName: register(b0, space0);
#endif

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
	ReadStructure Load(uint index, uint size = 0)
	{
		uint byteSize = size ? size : sizeof(ReadStructure);
		ByteAddressBuffer buffer = DESCRIPTOR_HEAP(ByteBufferHandle, this.handle.GetReadIndex());
		ReadStructure result = buffer.Load<ReadStructure>(byteSize * index);

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

float4 UnpackUnorm4x8(uint value)
{
    uint4 Packed = uint4(value & 0xff, (value >> 8) & 0xff, (value >> 16) & 0xff, value >> 24);
    return float4(Packed) / 255.0;
}      Assets/shaders/descriptorHeap.hlsl   p   �     #ifdef VK_BINDLESS
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
    [[vk::binding(1, 0)]] SamplerState g_SamplerState[];

    struct ByteBufferHandle { uint internalIndex; };

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

    	TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(Texture2D)
        TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(RWTexture2D)
    };

    static VulkanResourceDescriptorHeapInternal VkResourceDescriptorHeap;
    #define DESCRIPTOR_HEAP(handleType, handle) VkResourceDescriptorHeap[(handleType)handle]
    #define SAMPLER_HEAP(handle) g_SamplerState[NonUniformResourceIndex(handle)]
#else
    #define DESCRIPTOR_HEAP(handleType, handle) ResourceDescriptorHeap[NonUniformResourceIndex(handle)]
    #define SAMPLER_HEAP(handle) SamplerDescriptorHeap[NonUniformResourceIndex(handle)]
#endif     	   type.ByteAddressBuffer    
   g_ByteAddressBuffer  
    type.PushConstant.PushConstant           projection          vertices         pushConstants        out.var.COLOR0       out.var.TEXCOORD0        main    G        *   G            G            G           G  
   "       G  
   !       G           H  	       #       H  	          G  	      H         #       H               H            H        #   @   G        G     �              +                        +            +                     +          �?+        0   +           +           +           +           +                      	           	                                                       	         !            "           #            $      #     %            &      %     '   !  (   '      )   	         *   	         +         ;     
      ;         	   ;  !         ;  "         ;  $         ;  &              ,   6  '          (   �  -   =     .              &   A  )   /                 4   =     0   /        *   ]   S        0        &   >   �     1      .        &      �     2   1      A  +   3   
         2   =     4   3   |     5   4   �     6   2      A  +   7   
         6   =     8   7   |     9   8   �     :   2      A  +   ;   
         :   =     <   ;   |     =   <   �     >   1      �     ?   >      A  +   @   
         ?   =     A   @   |     B   A   �     C   1      �     D   C      A  +   E   
         D   =     F   E   |     G   F   �     H   1      �     I   H      A  +   J   
         I   =     K   J   |     L   K   �     M   I      A  +   N   
         M   =     O   N   |     P   O   �     Q   I      A  +   R   
         Q   =     S   R   |     T   S   P     U   L   P   T   ,              A  *   V                 &   =     W   V           8   P     X   5   9   =                 �     Y   X   W               O  #   Z   U   U                  !      P  %   [   B   G   = >     Y   >     Z   >     [        #      �  8  