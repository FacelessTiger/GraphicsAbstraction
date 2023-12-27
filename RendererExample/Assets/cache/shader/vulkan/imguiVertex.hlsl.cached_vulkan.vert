#      `              �    +     ,     1     �  
  SPV_EXT_descriptor_indexing      GLSL.std.450              	        main                 
    Assets/shaders/imguiVertex.hlsl  �    �     #define PushConstants	[[vk::offset(16)]] float2 scale; \
						[[vk::offset(24)]] float2 offset;
#include "bindless.hlsl"

struct ImDrawVert
{
	float2 pos;
	float2 uv;
	uint color;
};

struct Bindings
{
	ArrayBuffer vertices;
};

struct VertexOutput
{
	float4 position: SV_Position;
	float4 color: COLOR0;
	float2 uv: TEXCOORD0;
};

VertexOutput main(uint vertexID: SV_VertexID)
{
	Bindings bnd = loadBindings<Bindings>();
	ImDrawVert vertex = bnd.vertices.Load<ImDrawVert>(vertexID, 20);

	VertexOutput output;
	output.position = float4((vertex.pos * g_PushConstants.scale) + g_PushConstants.offset, 0, 1);
	output.color = UnpackUnorm4x8(vertex.color);
	output.uv = vertex.uv;
	return output;
}     
    Assets/shaders/bindless.hlsl     s   �     struct RenderResourceHandle
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
   type.ByteAddressBuffer       g_ByteAddressBuffer      type.PushConstant.PushConstantStruct             handle          scale           offset       RenderResourceHandle             index        g_PushConstants      out.var.COLOR0       out.var.TEXCOORD0        main    G        *   G            G            G           G     "       G     !       G           H  
       #       H  
          G  
      H         #       H         #       H        #      H        #      G        G     �              +                         +           +           +                    +            +          �?+            +           +           +           +           +           +         �     !                   
        "   
      #      "             $                 $   $      %   	         &            '      !      (      $     )   !  *   )      +   	   $      ,   	         -           .         ;  #         ;  %      	   ;  &         ;  '         ;  '         ;  (         +     /   ���;,  !   0   /   /   /   /   6  )          *   �  1   =     2           h   ,   A  ,   3                 h   C   =     4   3        i      A  -   5      4         =     6   5     	   *   Y   S        6           >   �     7      2              �     8   7      A  -   9            8   =     :   9   |     ;   :   �     <   8      A  -   =            <   =     >   =   |     ?   >   P  $   @   ;   ?   �     A   7      �     B   A      A  -   C            B   =     D   C   |     E   D   �     F   B      A  -   G            F   =     H   G   |     I   H   P  $   J   E   I   �     K   7      �     L   K      A  -   M            L   =     N   M           )   A  +   O                 9   =  $   P   O           B   A  +   Q                 R   =  $   R   Q           @     $   S      2   @   P   R              Q     T   S       Q     U   S      P  !   V   T   U              P       �     W   N            P   /   �     X   N           P   5   �     Y   X            P   D   �     Z   N           P   K   �     [   Z            P   Y   �     \   N           P      P  .   ]   W   Y   [   \        Q      p  !   ^   ]        Q      �  !   _   ^   0   = >     V   >     _   >     J        "      �  8  