#      �             �    +     ,     1     �  
  SPV_EXT_descriptor_indexing      GLSL.std.450                      main                      	     
   Assets/shaders/TriangleVertex.hlsl   )   �  
   #include "Cobra.hlsl"

struct Vertex
{
	float3 position;
	float uvX;
	float3 normal;
	float uvY;
	float4 color;
};

struct Object
{
	row_major float4x4 modelMatrix;
	uint vertices;
};

struct PushConstant
{
	float4x4 projection;
	float3 cameraPos;
	uint objects;
};
PushConstant(PushConstant, pushConstants);

struct VertexOutput
{
	float4 position: SV_Position;
	float3 worldPosition: POSITION0;
	float3 normal: NORMAL0;
	float3 color: COLOR0;
	float2 uv: TEXCOORD0;
};

VertexOutput main(uint vertexID: SV_VertexID, uint instanceIndex: SV_InstanceID)
{
	Object object = Cobra::ArrayBuffer::Create(pushConstants.objects).Load<Object>(instanceIndex);
	Vertex vertex = Cobra::ArrayBuffer::Create(object.vertices).Load<Vertex>(vertexID);

	float4 worldPos = mul(float4(vertex.position, 1.0f), object.modelMatrix);

	VertexOutput output;
	output.position = mul(pushConstants.projection, worldPos);
	output.worldPosition = worldPos.xyz;
	output.normal = mul(vertex.normal, (float3x3)transpose(Cobra::Inverse(object.modelMatrix)));
	output.color = vertex.color.xyz;
	output.uv = float2(vertex.uvX, vertex.uvY);
	return output;
}  
    Assets/shaders/Bindless.hlsl     f   �     #include "DescriptorHeap.hlsl"

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

}     	    Assets/shaders/Utils.hlsl    
   �     namespace Cobra {

	struct DrawIndexedIndirectCommand
	{
		uint reserved, reserved2;
		uint indexCount;
		uint instanceCount;
		uint firstIndex;
		uint vertexOffset;
		uint firstInstance;
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

}       Assets/shaders/DescriptorHeap.hlsl   �   �     namespace Cobra {

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

}      type.ByteAddressBuffer       g_ByteAddressBuffer  
    type.PushConstant.PushConstant           projection          cameraPos           objects      pushConstants        out.var.POSITION0        out.var.NORMAL0      out.var.COLOR0    	   out.var.TEXCOORD0        main    G        *   G        +   G            G            G           G           G  	         G     "       G     !       G           H         #       H            G        H         #       H               H            H        #   @   H        #   L   G        G     �  G     �              +           +                     +          �?             +        D   +           +            +           +           +         0   +     !      +     "      +     #      +     $                        %         &      %     '           (   '        )              (   )         *   	         +            ,      '      -      )     .            /      .     0   !  1   0      2   	         3   	   (     4   )         5         ;  &         ;  *      	   ;  +         ;  +         ;  ,         ;  -         ;  -         ;  -         ;  /   	      +     6      +     7      +     8   	   +     9   
   +     :      +     ;      +     <      +     =      +     >           ?   6  0          1   �  @   =     A      =     B        
   %   -   A  2   C           
   %   ;   =     D   C        .   a   S        D        .   $   �     E   B           .      �     F   E      A  5   G            F   =     H   G   |     I   H   �     J   F      A  5   K            J   =     L   K   |     M   L   �     N   F      A  5   O            N   =     P   O   |     Q   P   �     R   F   !   A  5   S            R   =     T   S   |     U   T   �     V   F   "   A  5   W            V   =     X   W   |     Y   X   �     Z   F   6   A  5   [            Z   =     \   [   |     ]   \   �     ^   F   7   A  5   _            ^   =     `   _   |     a   `   �     b   F   #   A  5   c            b   =     d   c   |     e   d   �     f   F   $   A  5   g            f   =     h   g   |     i   h   �     j   F   8   A  5   k            j   =     l   k   |     m   l   �     n   F   9   A  5   o            n   =     p   o   |     q   p   �     r   F   :   A  5   s            r   =     t   s   |     u   t   �     v   F   ;   A  5   w            v   =     x   w   |     y   x   �     z   F   <   A  5   {            z   =     |   {   |     }   |   �     ~   F   =   A  5               ~   =     �      |     �   �   �     �   F   >   A  5   �            �   =     �   �   |     �   �   P  '   �   I   M   Q   U   P  '   �   Y   ]   a   e   P  '   �   i   m   q   u   P  '   �   y   }   �   �   P  (   �   �   �   �   �   �     �   F      A  5   �            �   =     �   �        .   a   S        �        .   $   �     �   A            .      �     �   �      A  5   �            �   =     �   �   |     �   �   �     �   �      A  5   �            �   =     �   �   |     �   �   �     �   �      A  5   �            �   =     �   �   |     �   �   �     �   �   !   A  5   �            �   =     �   �   |     �   �   �     �   �   "   A  5   �            �   =     �   �   |     �   �   �     �   �   6   A  5   �            �   =     �   �   |     �   �   �     �   �   7   A  5   �            �   =     �   �   |     �   �   P  )   �   �   �   �   �     �   �   #   A  5   �            �   =     �   �   |     �   �   �     �   �   $   A  5   �            �   =     �   �   |     �   �   �     �   �   8   A  5   �            �   =     �   �   |     �   �   �     �   �   9   A  5   �            �   =     �   �   |     �   �   P  '   �   �   �   �   ?     
   (      P  '   �   �   �   �        
   (      �  '   �   �   �     
   +      A  3   �           
   +   &   =  (   �   �     
   +      �  '   �   �   �     
   ,      O  )   �   �   �                        �     �   m   �           (   �     �   }   q           .   �     �   �   e   =      �   �           "        �      2   �   e   �           :   �     �   }   a           4        �      2   �   u   �           L   �     �   ]   �   =      �   �           F        �      2   �   u   �           ^   �     �   m   a   =      �   �           X        �      2   �   �   �           p   �     �   ]   q           j        �      2   �   �   �              �     �   y   q           (   �     �   i   �           .   �     �   �   e   =      �   �           "        �      2   �   e   �           :   �     �   y   a   =      �   �           4        �      2   �   u   �           L   �     �   Y   �           F        �      2   �   u   �           ^   �     �   i   a           X        �      2   �   �   �           p   �     �   Y   q   =      �   �           j        �      2   �   �   �              �     �   i   }           (   �     �   y   m           .   �     �   �   e   =      �   �           "        �      2   �   e   �           :   �     �   y   ]           4        �      2   �   u   �           L   �     �   Y   }   =      �   �           F        �      2   �   u   �           ^   �     �   i   ]   =      �   �           X        �      2   �   �   �           p   �     �   Y   m           j        �      2   �   �   �           .   �     �   �   a   =      �   �           "        �      2   �   a   �   =      �   �           4        �      2   �   q   �           F        �      2   �   q   �           X        �      2   �   �   �   =      �   �           j        �      2   �   �   �           "   �     �   M   �                   �      2   I   �   �           (        �      2   Q   �   �           4        �      2   U   �   �               �     �      �        $      �     �   �   �        %   /   �     �   �   U   =         �        %   #             2   �   U           %   ;   �       }   Q   =               %   5             2     u          %   M   �       M   �        %   G             2     u          %   _   �       m   Q        %   Y             2     �          %   q   �     	  M   q   =      
  	       %   k             2   
  �          %   ~   �         �        &   /   �       �   U   =               &   #             2   �   U          &   5             2     e     =               &   G             2     e          &   _   �       ]   Q   =               &   Y             2     �          &   q   �       M   a        &   k             2     �          &   ~   �         �        '   /   �       �   U   =               '   #             2   �   U     =               '   5             2     e          '   G             2   	  e          '   Y             2     u     =                '   k        !     2      u          '   ~   �     "  !  �   = P  '   #  �       "       )      �     $  �   �        *   /   �     %  �   U   =      &  %       *   #        '     2   �   U   &       *   ;   �     (  y   Q        *   5        )     2   (  u   '       *   M   �     *  I   �   =      +  *       *   G        ,     2   +  u   )       *   _   �     -  i   Q   =      .  -       *   Y        /     2   .  �   ,       *   q   �     0  I   q        *   k        1     2   0  �   /       *   ~   �     2  1  �        +   /   �     3  �   U   =      4  3       +   #        5     2   �   U   4  =      6  (       +   5        7     2   6  e   5       +   G        8     2   *  e   7       +   _   �     9  Y   Q        +   Y        :     2   9  �   8       +   q   �     ;  I   a   =      <  ;       +   k        =     2   <  �   :       +   ~   �     >  =  �        ,   /   �     ?  �   U   =      @  ?       ,   #        A     2   �   U   @       ,   5        B     2   -  e   A  =      C  0       ,   G        D     2   C  e   B  =      E  9       ,   Y        F     2   E  u   D       ,   k        G     2   ;  u   F       ,   ~   �     H  G  �   = P  '   I  $  2  >  H       .      �     J  �   �        /   /   �     K  �   U   =      L  K       /   #        M     2   �   U   L       /   ;   �     N  y   M   =      O  N       /   5        P     2   O  u   M       /   M   �     Q  I   }        /   G        R     2   Q  u   P       /   _   �     S  i   M        /   Y        T     2   S  �   R       /   q   �     U  I   m   =      V  U       /   k        W     2   V  �   T       /   ~   �     X  W  �        0   /   �     Y  �   U   =      Z  Y       0   #        [     2   �   U   Z       0   5        \     2   N  e   [  =      ]  Q       0   G        ^     2   ]  e   \       0   _   �     _  Y   M   =      `  _       0   Y        a     2   `  �   ^       0   q   �     b  I   ]        0   k        c     2   b  �   a       0   ~   �     d  c  �        1   /   �     e  �   U   =      f  e       1   #        g     2   �   U   f  =      h  S       1   5        i     2   h  e   g       1   G        j     2   U  e   i       1   Y        k     2   _  u   j  =      l  b       1   k        m     2   l  u   k       1   ~   �     n  m  �   = P  '   o  J  X  d  n       3      �     p  �   �        4   /   �     q  �   Q   =      r  q       4   #        s     2   �   Q   r       4   5        t     2   N  q   s       4   G        u     2   ]  q   t       4   Y        v     2   h  �   u       4   k        w     2   U  �   v       4   ~   �     x  w  �        5   /   �     y  �   Q   =      z  y       5   #        {     2   �   Q   z       5   5        |     2   O  a   {       5   G        }     2   Q  a   |       5   Y        ~     2   _  �   }       5   k             2   l  �   ~       5   ~   �     �    �        6   /   �     �  �   Q   =      �  �       6   #        �     2   �   Q   �       6   5        �     2   S  a   �       6   G        �     2   V  a   �       6   Y        �     2   `  q   �       6   k        �     2   b  q   �       6   ~   �     �  �  �   = P  '   �  p  x  �  �  P  (   �  #  I  o  �    
   -   /   T  (   �  �  Q  '   �  �      O  )   �  �  �            Q  '   �  �     O  )   �  �  �            Q  '   �  �     O  )   �  �  �            P  4   �  �  �  �    
   -      �  )   �  �  �     
   .      O  )   �  �   �               
   /      P  .   �  �   �   = >     �   >     �   >     �  >     �  >  	   �    
   1      �  8  