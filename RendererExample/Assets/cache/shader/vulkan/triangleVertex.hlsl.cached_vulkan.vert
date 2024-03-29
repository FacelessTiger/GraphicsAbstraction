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
	ArrayBuffer vertices;
	ArrayBuffer modelMatrices;
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
	Vertex vertex = pushConstants.vertices.Load<Vertex>(vertexID);
	float4x4 modelMatrix = pushConstants.modelMatrices.Load<float4x4>(instanceIndex);

	VertexOutput output;
	output.position = mul(pushConstants.projection, mul(modelMatrix, float4(vertex.position, 1.0f)));
	output.color = vertex.color.xyz;
	output.uv = float2(vertex.uvX, vertex.uvY);
	return output;
}     
    Assets/shaders/bindless.hlsl     �   �     struct RenderResourceHandle
{
	uint index;
};

#include "descriptorHeap.hlsl"
#define PushConstant(structName, variableName) [[vk::push_constant]] ConstantBuffer<structName> variableName

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
}     	   Assets/shaders/descriptorHeap.hlsl   )   �  	   #define DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(textureType, bindingA, bindingB)                    \
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

#define DESCRIPTOR_HEAP(handleType, handle) VkResourceDescriptorHeap[(handleType)handle]      
   type.ByteAddressBuffer       g_ByteAddressBuffer  
    type.PushConstant.PushConstant           projection          vertices            modelMatrices        ArrayBuffer          handle       RenderResourceHandle             index        pushConstants        out.var.COLOR0       out.var.TEXCOORD0        main    G        *   G        +   G            G            G           G     "       G     !       G           H  
       #       H  
          G  
      H         #       H         #       H         #       H               H            H        #   @   H        #   D   G        G     �  G     �              +                        +            +           +                     +          �?+        0   +           +           +           +           +             +     !   @             
        "   
      #      "     $           %   $                           %            &   	         '            (      $     )            *      )     +            ,      +     -   !  .   -      /   	   %      0         ;  #         ;  &      	   ;  '         ;  '         ;  (         ;  *         ;  ,            1   	      +     2      +     3      +     4      +     5      +     6      +     7      +     8   	   +     9   
   +     :      +     ;      +     <      +     =           >   6  -          .   �  ?   =     @      =     A              I   A  1   B                       U   =     C   B     	   *   Y   S        C           >   �     D      @              �     E   D      A  0   F            E   =     G   F   |     H   G   �     I   E      A  0   J            I   =     K   J   |     L   K   �     M   E      A  0   N            M   =     O   N   |     P   O   �     Q   D      �     R   Q      A  0   S            R   =     T   S   |     U   T   �     V   D      �     W   V      A  0   X            W   =     Y   X   |     Z   Y   �     [   D       �     \   [      A  0   ]            \   =     ^   ]   |     _   ^   �     `   \      A  0   a            `   =     b   a   |     c   b   �     d   \      A  0   e            d   =     f   e   |     g   f   P  $   h   _   c   g   >           I   A  1   i                       U   =     j   i     	   *   Y   S        j           >   �     k   !   A              �     l   k      A  0   m            l   =     n   m   |     o   n   �     p   l      A  0   q            p   =     r   q   |     s   r   �     t   l      A  0   u            t   =     v   u   |     w   v   �     x   l   2   A  0   y            x   =     z   y   |     {   z   �     |   l   3   A  0   }            |   =     ~   }   |        ~   �     �   l   4   A  0   �            �   =     �   �   |     �   �   �     �   l   5   A  0   �            �   =     �   �   |     �   �   �     �   l   6   A  0   �            �   =     �   �   |     �   �   �     �   l   7   A  0   �            �   =     �   �   |     �   �   �     �   l   8   A  0   �            �   =     �   �   |     �   �   �     �   l   9   A  0   �            �   =     �   �   |     �   �   �     �   l   :   A  0   �            �   =     �   �   |     �   �   �     �   l      A  0   �            �   =     �   �   |     �   �   �     �   l   ;   A  0   �            �   =     �   �   |     �   �   �     �   l   <   A  0   �            �   =     �   �   |     �   �   �     �   l   =   A  0   �            �   =     �   �   |     �   �   P  $   �   o      �   �   P  $   �   s   �   �   �   P  $   �   w   �   �   �   P  $   �   {   �   �   �   P  %   �   �   �   �   �        !      A  /   �              !   &   =  %   �   �        !   I   P  $   �   H   L   P           !   2   �  $   �   �   �        !      �  $   �   �   �        "      O  )   �   h   h                  #      P  +   �   U   Z   = >     �   >     �   >     �        %      �  8  