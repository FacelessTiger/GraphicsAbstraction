#      �              �    +     ,     1     �  
  SPV_EXT_descriptor_indexing           
        main                        Assets/shaders/triangleVertex.hlsl   �    �     #include "bindless.hlsl"

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
	uint modelMatrices;
};
PushConstant(PushConstant, pushConstants);

struct VertexOutput
{
	float4 position: SV_Position;
	float3 color: COLOR0;
	float2 uv: TEXCOORD0;
};

VertexOutput main(uint vertexID: SV_VertexID, uint instanceIndex: SV_InstanceID)
{
	Vertex vertex = ArrayBuffer::Create(pushConstants.vertices).Load<Vertex>(vertexID);
	float4x4 modelMatrix = ArrayBuffer::Create(pushConstants.modelMatrices).Load<float4x4>(instanceIndex);

	VertexOutput output;
	output.position = mul(pushConstants.projection, mul(float4(vertex.position, 1.0f), modelMatrix));
	output.color = vertex.color.xyz;
	output.uv = float2(vertex.uvX, vertex.uvY);
	return output;
}     
    Assets/shaders/bindless.hlsl     \   �     struct RenderResourceHandle
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

	struct SpecialConstant
	{
		uint vertexOffset;
		uint instanceOffset;
	};
	ConstantBuffer<SpecialConstant> g_SpecialConstants: register(b1, space0);
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

float4 UnpackUnorm4x8(uint value)
{
    uint4 Packed = uint4(value & 0xff, (value >> 8) & 0xff, (value >> 16) & 0xff, value >> 24);
    return float4(Packed) / 255.0;
}     	   Assets/shaders/descriptorHeap.hlsl   p   �  	   #ifdef VK_BINDLESS
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
#endif     
   type.ByteAddressBuffer       g_ByteAddressBuffer  
    type.PushConstant.PushConstant           projection          vertices            modelMatrices        pushConstants        out.var.COLOR0       out.var.TEXCOORD0        main    G        *   G        +   G            G            G           G     "       G     !       G           H  
       #       H  
          G  
      H         #       H               H            H        #   @   H        #   D   G        G     �  G     �              +           +           +                     +          �?             +        0   +           +            +           +           +           +           +           +         @             
        !   
      "      !     #           $   #           $            %   	         &            '      #     (            )      (     *            +      *     ,   !  -   ,      .   	         /   	   $      0         ;  "         ;  %      	   ;  &         ;  &         ;  '         ;  )         ;  +         +     1      +     2      +     3   	   +     4   
   +     5      +     6      +     7      +     8      +     9           :   6  ,          -   �  ;   =     <      =     =              &   A  .   >                 4   =     ?   >     	   *   ]   S        ?        ,   ;   �     @   <           ,      �     A   @      A  0   B            A   =     C   B   |     D   C   �     E   A      A  0   F            E   =     G   F   |     H   G   �     I   A      A  0   J            I   =     K   J   |     L   K   �     M   A      A  0   N            M   =     O   N   |     P   O   �     Q   A      A  0   R            Q   =     S   R   |     T   S   �     U   A      A  0   V            U   =     W   V   |     X   W   �     Y   A   3   A  0   Z            Y   =     [   Z   |     \   [   �     ]   A   4   A  0   ^            ]   =     _   ^   |     `   _   P  #   a   X   \   `   :           -   A  .   b                 ;   =     c   b     	   *   ]   S        c        ,   ;   �     d   =            ,      �     e   d      A  0   f            e   =     g   f   |     h   g   �     i   e      A  0   j            i   =     k   j   |     l   k   �     m   e      A  0   n            m   =     o   n   |     p   o   �     q   e      A  0   r            q   =     s   r   |     t   s   �     u   e      A  0   v            u   =     w   v   |     x   w   �     y   e   1   A  0   z            y   =     {   z   |     |   {   �     }   e   2   A  0   ~            }   =        ~   |     �      �     �   e      A  0   �            �   =     �   �   |     �   �   �     �   e      A  0   �            �   =     �   �   |     �   �   �     �   e   3   A  0   �            �   =     �   �   |     �   �   �     �   e   4   A  0   �            �   =     �   �   |     �   �   �     �   e   5   A  0   �            �   =     �   �   |     �   �   �     �   e   6   A  0   �            �   =     �   �   |     �   �   �     �   e   7   A  0   �            �   =     �   �   |     �   �   �     �   e   8   A  0   �            �   =     �   �   |     �   �   �     �   e   9   A  0   �            �   =     �   �   |     �   �   P  #   �   h   l   p   t   P  #   �   x   |   �   �   P  #   �   �   �   �   �   P  #   �   �   �   �   �   P  $   �   �   �   �   �        !      A  /   �              !   &   =  $   �   �        !   <   P  #   �   D   H   L           !   2   �  #   �   �   �        !      �  #   �   �   �        "      O  (   �   a   a                  #      P  *   �   P   T   = >     �   >     �   >     �        %      �  8  