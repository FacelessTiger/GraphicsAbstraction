#      �              �    +     ,     1     �  
  SPV_EXT_descriptor_indexing      GLSL.std.450              
        main                    
    Assets/shaders/quadVertex.hlsl   s   �     #include "bindless.hlsl"

struct QuadData
{
	float2 scale;
	Texture texture;
	uint color;
	float3 position;
	float rotation;
};

struct PushConstant
{
	float4x4 projection;
	ArrayBuffer quadData;
	Sampler sampler;
};
PushConstant(PushConstant, pushConstants);

struct VertexOutput
{
	float4 position: SV_Position;
	float4 color: COLOR0;
	Texture texture: BLENDINDICES0;
	float2 uv: TEXCOORD0;
};

VertexOutput main(uint vertexID: SV_VertexID)
{
	const float2 vertices[6] = {
		float2( 0.5f, -0.5f),
		float2( 0.5f,  0.5f),
		float2(-0.5f, -0.5f),
		float2(-0.5f, -0.5f),
		float2( 0.5f,  0.5f),
		float2(-0.5f,  0.5f)
	};

	const float2 uvs[6] = {
		float2(1, 0),
		float2(1, 1),
		float2(0, 0),
		float2(0, 0),
		float2(1, 1),
		float2(0, 1)
	};

	int quadID = vertexID / 6;
	int relativeVertexID = vertexID % 6;

	QuadData quad = pushConstants.quadData.Load<QuadData>(quadID);
	float2 pos = vertices[relativeVertexID];

	float2 cossin = float2(cos(quad.rotation), sin(quad.rotation));
	float2 rotatedPos = float2(pos.x * cossin.x - pos.y * cossin.y, pos.x * cossin.y + pos.y * cossin.x);
	float2 relativePos = rotatedPos * quad.scale + quad.position.xy;

	VertexOutput output;
	output.position = mul(pushConstants.projection, float4(relativePos, quad.position.z, 1.0f));
	output.color = UnpackUnorm4x8(quad.color);
	output.texture = quad.texture;
	output.uv = uvs[relativeVertexID];
	return output;
}    
 	   Assets/shaders/bindless.hlsl     �   �  	   struct RenderResourceHandle
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
}     
   Assets/shaders/descriptorHeap.hlsl   )   �  
   #define DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(textureType, bindingA, bindingB)                    \
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

#define DESCRIPTOR_HEAP(handleType, handle) VkResourceDescriptorHeap[(handleType)handle]         type.ByteAddressBuffer       g_ByteAddressBuffer  
    type.PushConstant.PushConstant           projection          quadData            sampler      ArrayBuffer          handle       RenderResourceHandle             index        Sampler          handle       pushConstants        out.var.COLOR0       out.var.BLENDINDICES0        out.var.TEXCOORD0        main    G        *   G            G        G            G           G           G     "       G     !       G           H         #       H            G        H         #       H         #       H         #       H         #       H               H            H        #   @   H        #   D   G        G     �           +           ?+          �?+                         +                       +           +            +            +            +           +            +     !      +     "      +     #      +     $      +     %   �   +     &        '                           (         )      (     *   '                                   *            +   	         ,            -      '      .           /            0      /     1   !  2   1     3   /         4      3     5            6      /      7   	   *      8           9         ;  )         ;  +      	   ;  ,         ;  -         ;  -         ;  .         ;  0            :   	      +     ;      �,  /   <      ;   ,  /   =         ,  /   >   ;   ;   ,  /   ?   ;      , 	 3   @   <   =   >   >   =   ?   ,  /   A         ,  /   B         ,  /   C         ,  /   D         , 	 3   E   A   B   C   C   B   D   +     F   ���;,  '   G   F   F   F   F   6  1          2   �  H   ;  4   I      ;  4   J      =     K                 >  I   @        '      >  J   E        0      �     L   K           0      |     M   L        1   "   �     N   K           1      |     O   N        3   8   |     P   M     	      I   A  :   Q                 	      U   =     R   Q     
   *   Y   S        R     	      >   �     S      P     	         �     T   S      A  8   U            T   =     V   U   |     W   V   �     X   T       A  8   Y            X   =     Z   Y   |     [   Z   P  /   \   W   [   �     ]   S   !   �     ^   ]      A  8   _            ^   =     `   _   �     a   S   "   �     b   a      A  8   c            b   =     d   c   �     e   S   #   �     f   e      A  8   g            f   =     h   g   |     i   h   �     j   f       A  8   k            j   =     l   k   |     m   l   �     n   f      A  8   o            n   =     p   o   |     q   p   P  5   r   i   m   q   �     s   S   $   �     t   s      A  8   u            t   =     v   u   |     w   v        4      A  6   x   I   O   =  /   y   x        6           z         w        6   -        {         w        7      Q     |   y            7   0   Q     }   y           7   6   �     ~   }   {   =         ~        7   .        �      2   |   z           7   [   �     �   }   z        7   S        �      2   |   {   �        7      P  /   �   �   �        8   1   O  /   �   r   r               8   /     /   �      2   �   \   �        ;      A  7   �              ;   &   =  *   �   �        ;   8   Q     �   �       Q     �   �      P  '   �   �   �   q           ;      �  '   �   �   �     	   Q       �     �   d   %     	   Q   /   �     �   d   !     	   Q   5   �     �   �   %     	   Q   D   �     �   d   #     	   Q   K   �     �   �   %     	   Q   Y   �     �   d   &     	   Q      P  9   �   �   �   �   �     	   R      p  '   �   �     	   R      �  '   �   �   G        >      A  6   �   J   O   =  /   �   �   = >     �   >     �   >     `   >     �        @      �  8  