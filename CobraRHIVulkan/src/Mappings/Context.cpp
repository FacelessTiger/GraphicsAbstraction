#include <VulkanRHI.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <iostream>

namespace GraphicsAbstraction {

	Ref<Impl<GraphicsContext>> Impl<GraphicsContext>::Reference;

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		switch (messageSeverity)
		{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:	std::cout << pCallbackData->pMessage << std::endl; break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:		std::cout << pCallbackData->pMessage << std::endl; break;
		}

		return false;
	}

	void GraphicsContext::Init(uint32_t frameInFlightCount)
	{
		Impl<GraphicsContext>::Reference = CreateRef<Impl<GraphicsContext>>(frameInFlightCount);
	}

	Ref<Queue> GraphicsContext::GetQueue(QueueType type)
	{
		auto& reference = *Impl<GraphicsContext>::Reference;
		auto queue = CreateRef<Queue>();

		switch (type)
		{
			case QueueType::Graphics: queue->impl = new Impl<Queue>(reference, reference.GraphicsQueue, reference.GraphicsQueueFamily); break;
			default: assert(false && "Unkown queue type!");
		}

		return queue;
	}

	ShaderCompiledType GraphicsContext::GetShaderCompiledType()
	{
		return ShaderCompiledType::Spirv;
	}

	void GraphicsContext::SetFrameInFlight(uint32_t fif)
	{
		auto impl = Impl<GraphicsContext>::Reference;
		impl->FrameInFlight = fif;
		impl->FrameDeletionQueues[fif].Flush();
	}

	Impl<GraphicsContext>::Impl(uint32_t frameInFlightCount)
	{
		//m_RefCount--; // Kinda cheating, but its okey ;P
		SetupInstance();
		SetupPhysicalDevice();
		SetupLogicalDevice();

		VmaAllocatorCreateInfo allocatorInfo = {
			.physicalDevice = ChosenGPU,
			.device = Device,
			.instance = Instance
		};
		vmaCreateAllocator(&allocatorInfo, &Allocator);

		SetupBindless();

#define GA_VULKAN_FUNCTION(name) {												\
				auto pfn = (PFN_##name)vkGetDeviceProcAddr(Device, #name);		\
				if (pfn) name = pfn;											\
		}
#include "../InternalManagers/VulkanFunctions.inl"

		FrameDeletionQueues.resize(frameInFlightCount, *this);
		if (!g_ShaderObjectSupported) PipelineManager = new GraphicsAbstraction::PipelineManager(*this);
		if (!g_DynamicRenderingSupported) RenderInfoManager = new GraphicsAbstraction::RenderInfoManager(*this);
	}

	Impl<GraphicsContext>::~Impl()
	{
		vkDeviceWaitIdle(Device);

		for (auto& deletionQueue : FrameDeletionQueues)
			deletionQueue.Flush();
		if (!g_ShaderObjectSupported) delete PipelineManager;
		if (!g_DynamicRenderingSupported) delete RenderInfoManager;

		vkDestroyDescriptorPool(Device, BindlessPool, nullptr);
		vkDestroyDescriptorSetLayout(Device, BindlessSetLayout, nullptr);
		vkDestroyPipelineLayout(Device, BindlessPipelineLayout, nullptr);

		vmaDestroyAllocator(Allocator);

		vkDestroyDevice(Device, nullptr);
		vkDestroyDebugUtilsMessengerEXT(Instance, DebugMessenger, nullptr);
		vkDestroyInstance(Instance, nullptr);
	}

	void Impl<GraphicsContext>::SetupInstance()
	{
		VkApplicationInfo appInfo = {
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pEngineName = "Cobra",
			.apiVersion = VK_API_VERSION_1_2
		};

		std::vector<const char*> deviceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_win32_surface" };
		std::vector<const char*> enabledLayers;
#ifndef GA_DIST
		deviceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif

		VkInstanceCreateInfo instanceCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pApplicationInfo = &appInfo,
			.enabledLayerCount = (uint32_t)enabledLayers.size(),
			.ppEnabledLayerNames = enabledLayers.data(),
			.enabledExtensionCount = (uint32_t)deviceExtensions.size(),
			.ppEnabledExtensionNames = deviceExtensions.data()
		};
		VK_CHECK(vkCreateInstance(&instanceCreateInfo, nullptr, &Instance));

#define GA_VULKAN_FUNCTION(name) {												\
				auto pfn = (PFN_##name)vkGetInstanceProcAddr(Instance, #name);	\
				if (pfn) name = pfn;											\
		}
#include "../InternalManagers/VulkanFunctions.inl"

#ifndef GA_DIST
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
			.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
			.pfnUserCallback = debugCallback,
		};
		vkCreateDebugUtilsMessengerEXT(Instance, &debugCreateInfo, nullptr, &DebugMessenger);
#endif
	}

	void Impl<GraphicsContext>::SetupPhysicalDevice()
	{
		// Get devices
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(Instance, &deviceCount, nullptr);
		assert(deviceCount && "Could not find device that supports vulkan!");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(Instance, &deviceCount, devices.data());

		// Check features and select
		for (const auto& device : devices)
		{
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);

			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				ChosenGPU = device;
				break;
			}
		}

		// Check optional extensions
		uint32_t propertyCount;
		vkEnumerateDeviceExtensionProperties(ChosenGPU, nullptr, &propertyCount, nullptr);
		std::vector<VkExtensionProperties> properties(propertyCount);
		vkEnumerateDeviceExtensionProperties(ChosenGPU, nullptr, &propertyCount, properties.data());

		for (auto& property : properties)
		{
			// Shader object requires dynamic rendering, and we can act as if dynamic state 1-3 is enabled since it uses the same commands
			if (!std::strcmp(property.extensionName, VK_EXT_SHADER_OBJECT_EXTENSION_NAME))
			{
				g_ShaderObjectSupported = true;
				g_DynamicStateSupported = true;
				g_DynamicState2Supported = true;
				g_DynamicState3Supported = true;
				g_DynamicRenderingSupported = true;
			}

			if (!std::strcmp(property.extensionName, VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME)) g_DynamicStateSupported = true;
			if (!std::strcmp(property.extensionName, VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME)) g_DynamicState2Supported = true;
			if (!std::strcmp(property.extensionName, VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME)) g_DynamicState3Supported = true;
			if (!std::strcmp(property.extensionName, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)) g_DynamicRenderingSupported = true;
		}
	}

	void Impl<GraphicsContext>::SetupLogicalDevice()
	{
		// Choose and setup queues
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(ChosenGPU, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(ChosenGPU, &queueFamilyCount, queueFamilies.data());

		for (int i = 0; i < queueFamilies.size(); i++)
		{
			auto& queueFamily = queueFamilies[i];
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				GraphicsQueueFamily = i;
			}
		}

		float queuePriority = 1.0f;
		VkDeviceQueueCreateInfo queueCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = GraphicsQueueFamily,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority
		};

		// Setup device features and create
		Utils::FeatureBuilder builder;
		builder.AddFeature<VkPhysicalDeviceVulkan12Features>({
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
			.drawIndirectCount = true,
			.shaderSampledImageArrayNonUniformIndexing = true,
			.shaderStorageImageArrayNonUniformIndexing = true,
			.descriptorBindingSampledImageUpdateAfterBind = true,
			.descriptorBindingStorageImageUpdateAfterBind = true,
			.descriptorBindingStorageBufferUpdateAfterBind = true,
			.descriptorBindingPartiallyBound = true,
			.descriptorBindingVariableDescriptorCount = true,
			.runtimeDescriptorArray = true,
			.imagelessFramebuffer = !g_DynamicRenderingSupported,
			.separateDepthStencilLayouts = !g_DynamicRenderingSupported,
			.timelineSemaphore = true
		});

		builder.AddExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		if (g_ShaderObjectSupported)
		{
			builder.AddFeature<VkPhysicalDeviceShaderObjectFeaturesEXT>({
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT,
				.shaderObject = true
			});
			builder.AddExtension(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
		}
		else
		{
			if (g_DynamicStateSupported)
			{
				builder.AddFeature<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT>({
					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
					.extendedDynamicState = true
				});
				builder.AddExtension(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
			}

			if (g_DynamicState2Supported)
			{
				builder.AddFeature<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT>({
					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
					.extendedDynamicState2 = true
				});
				builder.AddExtension(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
			}

			if (g_DynamicState3Supported)
			{
				builder.AddFeature<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>({
					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT,
					.extendedDynamicState3PolygonMode = true,
					.extendedDynamicState3RasterizationSamples = true,
					.extendedDynamicState3SampleMask = true,
					.extendedDynamicState3AlphaToCoverageEnable = true,
					.extendedDynamicState3ColorBlendEnable = true,
					.extendedDynamicState3ColorBlendEquation = true,
					.extendedDynamicState3ColorWriteMask = true
				});
				builder.AddExtension(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
			}
		}

		if (g_DynamicRenderingSupported)
		{
			builder.AddFeature<VkPhysicalDeviceDynamicRenderingFeatures>({
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
				.dynamicRendering = true
			});
			builder.AddExtension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
		}

		builder.AddFeature<VkPhysicalDeviceFeatures2>({
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.features = {
				.multiDrawIndirect = true,
				.fillModeNonSolid = true
			}
		});

		VkDeviceCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = builder.GetChain(),
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &queueCreateInfo,
			.enabledExtensionCount = (uint32_t)builder.extensions.size(),
			.ppEnabledExtensionNames = builder.extensions.data()
		};

		VkResult result = vkCreateDevice(ChosenGPU, &createInfo, nullptr, &Device);
		assert((result == VK_SUCCESS) && (std::string("Failed to create device with vulkan error ") + string_VkResult(result)).c_str());

		vkGetDeviceQueue(Device, GraphicsQueueFamily, 0, &GraphicsQueue);
	}

	void Impl<GraphicsContext>::SetupBindless()
	{
		uint32_t maxDescriptors = 100000;
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(ChosenGPU, &properties);

		struct BindingInfo
		{
			VkDescriptorType Type;
			uint32_t Count;
			uint32_t Binding;
		};

		const auto bindingInfos = std::array{
			BindingInfo { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, properties.limits.maxDescriptorSetStorageBuffers, STORAGE_BINDING },
			BindingInfo { VK_DESCRIPTOR_TYPE_SAMPLER, properties.limits.maxDescriptorSetSamplers, SAMPLER_BINDING },
			BindingInfo { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, properties.limits.maxDescriptorSetStorageImages, STORAGE_IMAGE_BINDING },
			BindingInfo { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, properties.limits.maxDescriptorSetSampledImages, SAMPLED_IMAGE_BINDING }
		};

		std::vector<VkDescriptorPoolSize> poolSizes;
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		std::vector<VkDescriptorBindingFlags> bindingFlags;

		for (auto& bindingInfo : bindingInfos)
		{
			poolSizes.push_back({ bindingInfo.Type, bindingInfo.Count });
			bindings.push_back({
				.binding = bindingInfo.Binding,
				.descriptorType = bindingInfo.Type,
				.descriptorCount = bindingInfo.Count,
				.stageFlags = VK_SHADER_STAGE_ALL
			});

			bindingFlags.push_back(VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
		}

		VkDescriptorPoolCreateInfo poolInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
			.maxSets = 1,
			.poolSizeCount = (uint32_t)poolSizes.size(),
			.pPoolSizes = poolSizes.data()
		};
		VK_CHECK(vkCreateDescriptorPool(Device, &poolInfo, nullptr, &BindlessPool));

		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
			.bindingCount = (uint32_t)bindingFlags.size(),
			.pBindingFlags = bindingFlags.data()
		};

		VkDescriptorSetLayoutCreateInfo layoutInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = &bindingFlagsInfo,
			.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
			.bindingCount = (uint32_t)bindings.size(),
			.pBindings = bindings.data()
		};
		VK_CHECK(vkCreateDescriptorSetLayout(Device, &layoutInfo, nullptr, &BindlessSetLayout));

		VkDescriptorSetAllocateInfo setInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = BindlessPool,
			.descriptorSetCount = 1,
			.pSetLayouts = &BindlessSetLayout
		};
		VK_CHECK(vkAllocateDescriptorSets(Device, &setInfo, &BindlessSet));

		VkPipelineLayoutCreateInfo pipelineInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 1,
			.pSetLayouts = &BindlessSetLayout,
			.pushConstantRangeCount = (uint32_t)PUSH_CONSTANT_RANGES.size(),
			.pPushConstantRanges = PUSH_CONSTANT_RANGES.data()
		};
		VK_CHECK(vkCreatePipelineLayout(Device, &pipelineInfo, nullptr, &BindlessPipelineLayout));
	}

}