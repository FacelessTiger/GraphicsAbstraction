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