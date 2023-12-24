#include "VulkanContext.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <GLFW/glfw3.h>
#include <GraphicsAbstraction/Core/Log.h>
#include <GraphicsAbstraction/Debug/Instrumentor.h>
#include <GraphicsAbstraction/Core/Assert.h>
#include <Platform/GraphicsAPI/Vulkan/VulkanQueue.h>
#include <Platform/GraphicsAPI/Vulkan/VulkanShader.h>
#include <Platform/GraphicsAPI/Vulkan/VulkanUtils.h>

#include <variant>

namespace GraphicsAbstraction {

	VulkanContext* VulkanContext::s_Instance = nullptr;

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		switch (messageSeverity)
		{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:	GA_CORE_WARN("{0}", pCallbackData->pMessage); break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:		GA_CORE_ERROR("{0}", pCallbackData->pMessage); break;
			//case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:		GA_CORE_INFO("{0}", pCallbackData->pMessage); break;
		}

		return false;
	}

	VulkanContext::VulkanContext(uint32_t frameInFlightCount)
	{
		s_Instance = this;
		
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
		SetupFunctions();

		FrameDeletionQueues.resize(frameInFlightCount, *this);

		if (!ShaderObjectSupported)
		{
			std::vector<uint8_t> fileData;
			std::ifstream file("Assets/cache/pipelineCache.vulkan", std::ios::binary);

			if (file.is_open())
			{
				std::streampos fileSize;

				file.seekg(0, std::ios::end);
				fileSize = file.tellg();
				file.seekg(0, std::ios::beg);

				fileData.resize(fileSize);
				file.read((char*)fileData.data(), fileSize);
			}

			VkPipelineCacheCreateInfo cacheInfo = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
				.initialDataSize = fileData.size(),
				.pInitialData = fileData.data()
			};
			VK_CHECK(vkCreatePipelineCache(Device, &cacheInfo, nullptr, &m_PipelineCache));
		}
	}

	std::shared_ptr<GraphicsAbstraction::Queue> VulkanContext::GetQueueImpl(QueueType type)
	{
		switch (type)
		{
			case QueueType::Graphics: return std::make_shared<VulkanQueue>(*this, GraphicsQueue, GraphicsQueueFamily);
		}

		GA_CORE_ASSERT(false, "Unknown queue type!"); 
		return nullptr;
	}

	VkPipeline VulkanContext::GetGraphicsPipeline(const VulkanGraphicsPipelineKey& key)
	{
		GA_PROFILE_SCOPE();
		if (m_GraphicsPipelines.find(key) != m_GraphicsPipelines.end()) return m_GraphicsPipelines[key];

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		for (uint32_t id : key.Shaders) shaderStages.push_back(VulkanShader::GetShaderByID(id)->StageInfo);

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		VkPipelineViewportStateCreateInfo viewportStateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		VkPipelineRasterizationStateCreateInfo rasterizationInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, .polygonMode = VK_POLYGON_MODE_FILL };
		VkPipelineDepthStencilStateCreateInfo depthInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = { 
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, 
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
		};

		VkPipelineMultisampleStateCreateInfo multisampleInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.minSampleShading = 1.0f,
			.pSampleMask = nullptr,
			.alphaToCoverageEnable = VK_FALSE,
			.alphaToOneEnable = VK_FALSE
		};

		VkPipelineColorBlendAttachmentState colorAttachment = {
			.blendEnable = VK_FALSE,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
		};
		VkPipelineColorBlendStateCreateInfo colorBlendInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = 1,
			.pAttachments = &colorAttachment
		};

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT, VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT, VK_DYNAMIC_STATE_LINE_WIDTH, VK_DYNAMIC_STATE_BLEND_CONSTANTS,
			VK_DYNAMIC_STATE_DEPTH_BOUNDS, VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK, VK_DYNAMIC_STATE_STENCIL_WRITE_MASK, VK_DYNAMIC_STATE_STENCIL_REFERENCE,
			VK_DYNAMIC_STATE_CULL_MODE, VK_DYNAMIC_STATE_FRONT_FACE, VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY, VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
			VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE, VK_DYNAMIC_STATE_DEPTH_COMPARE_OP, VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE, VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE,
			VK_DYNAMIC_STATE_STENCIL_OP, VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE, VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE, VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE
		};
		if (DynamicState3Supported) dynamicStates.insert(dynamicStates.end(), {
			VK_DYNAMIC_STATE_POLYGON_MODE_EXT, VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT, VK_DYNAMIC_STATE_SAMPLE_MASK_EXT, VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT,
			VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT, VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT
		});

		VkPipelineDynamicStateCreateInfo dynamicInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = (uint32_t)dynamicStates.size(),
			.pDynamicStates = dynamicStates.data()
		};

		VkPipelineRenderingCreateInfo renderingInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			.colorAttachmentCount = (uint32_t)key.ColorAttachments.size(),
			.pColorAttachmentFormats = key.ColorAttachments.data(),
			.depthAttachmentFormat = key.DepthAttachment
		};

		VkGraphicsPipelineCreateInfo pipelineInfo = {
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = &renderingInfo,
			.stageCount = (uint32_t)shaderStages.size(),
			.pStages = shaderStages.data(),
			.pVertexInputState = &vertexInputInfo,
			.pInputAssemblyState = &inputAssemblyInfo,
			.pViewportState = &viewportStateInfo,
			.pRasterizationState = &rasterizationInfo,
			.pMultisampleState = &multisampleInfo,
			.pDepthStencilState = &depthInfo,
			.pColorBlendState = &colorBlendInfo,
			.pDynamicState = &dynamicInfo,
			.layout = BindlessPipelineLayout
		};

		VkPipeline pipeline;
		VK_CHECK(vkCreateGraphicsPipelines(Device, m_PipelineCache, 1, &pipelineInfo, nullptr, &pipeline));
		m_GraphicsPipelines[key] = pipeline;

		return pipeline;
	}

	VkPipeline VulkanContext::GetComputePipeline(const VulkanComputePipelineKey& key)
	{
		GA_PROFILE_SCOPE();
		if (m_ComputePipelines.find(key) != m_ComputePipelines.end()) return m_ComputePipelines[key];

		VkComputePipelineCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.stage = VulkanShader::GetShaderByID(key.Shader)->StageInfo,
			.layout = BindlessPipelineLayout
		};

		VkPipeline pipeline;
		VK_CHECK(vkCreateComputePipelines(Device, m_PipelineCache, 1, &createInfo, nullptr, &pipeline));
		m_ComputePipelines[key] = pipeline;

		return pipeline;
	}

	void VulkanContext::SetupInstance()
	{
		VkApplicationInfo appInfo = {
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pEngineName = "Cobra",
			.apiVersion = VK_API_VERSION_1_3
		};

		uint32_t glfwExtensionCount;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> deviceExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

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

		if (VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &Instance))
		{
			GA_CORE_FATAL("Failed to create instance with vulkan error {}!", string_VkResult(result));
			GA_CORE_ASSERT(false);
		}

#define GA_VULKAN_FUNCTION(name) {												\
				auto pfn = (PFN_##name)vkGetInstanceProcAddr(Instance, #name);	\
				if (pfn) name = pfn;											\
		}
#include "VulkanFunctions.inl"

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

	void VulkanContext::SetupPhysicalDevice()
	{
		// Get devices
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(Instance, &deviceCount, nullptr);

		if (!deviceCount)
		{
			GA_CORE_FATAL("Could not find device that supports vulkan!");
			GA_CORE_ASSERT(false);
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(Instance, &deviceCount, devices.data());

		// Check features and select
		for (const auto& device : devices)
		{
			// for now just select first device
			ChosenGPU = device;
			break;
		}

		// Check optional extensions
		uint32_t propertyCount;
		vkEnumerateDeviceExtensionProperties(ChosenGPU, nullptr, &propertyCount, nullptr);
		std::vector<VkExtensionProperties> properties(propertyCount);
		vkEnumerateDeviceExtensionProperties(ChosenGPU, nullptr, &propertyCount, properties.data());

		// TODO: we should also check for the dynamic state extensions themselves
		for (auto& property : properties)
		{
			if (!std::strcmp(property.extensionName, VK_EXT_SHADER_OBJECT_EXTENSION_NAME))
			{
				ShaderObjectSupported = true;
				DynamicState3Supported = true;
			}

			if (!std::strcmp(property.extensionName, VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME)) DynamicState3Supported = true;
		}
	}

	void VulkanContext::SetupLogicalDevice()
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
		Utils::VulkanFeatureBuilder builder;
		builder.AddFeature<VkPhysicalDeviceVulkan12Features>({
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
			.shaderSampledImageArrayNonUniformIndexing = true,
			.shaderStorageImageArrayNonUniformIndexing = true,
			.descriptorBindingSampledImageUpdateAfterBind = true,
			.descriptorBindingStorageImageUpdateAfterBind = true,
			.descriptorBindingStorageBufferUpdateAfterBind = true,
			.descriptorBindingPartiallyBound = true,
			.descriptorBindingVariableDescriptorCount = true,
			.runtimeDescriptorArray = true,
			.timelineSemaphore = true
		});

		builder.AddFeature<VkPhysicalDeviceVulkan13Features>({
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
			.synchronization2 = true,
			.dynamicRendering = true
		});

		builder.AddExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		if (ShaderObjectSupported)
		{
			builder.AddFeature<VkPhysicalDeviceShaderObjectFeaturesEXT>({
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT,
				.shaderObject = true
			});
			builder.AddExtension(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
		}
		else
		{
			if (DynamicState3Supported)
			{
				builder.AddFeature<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>({
					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT,
					.extendedDynamicState3PolygonMode = true,
					.extendedDynamicState3RasterizationSamples = true,
					.extendedDynamicState3SampleMask = true,
					.extendedDynamicState3AlphaToCoverageEnable = true,
					.extendedDynamicState3ColorBlendEnable = true,
					.extendedDynamicState3ColorWriteMask = true
				});
				builder.AddExtension(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
			}
		}

		VkDeviceCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = builder.Build(),
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &queueCreateInfo,
			.enabledExtensionCount = (uint32_t)builder.extensions.size(),
			.ppEnabledExtensionNames = builder.extensions.data()
		};

		if (VkResult result = vkCreateDevice(ChosenGPU, &createInfo, nullptr, &Device))
		{
			GA_CORE_FATAL("Failed to create device with vulkan error {}!", string_VkResult(result));
			GA_CORE_ASSERT(false);
		}

		vkGetDeviceQueue(Device, GraphicsQueueFamily, 0, &GraphicsQueue);
	}

	void VulkanContext::SetupBindless()
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

		const auto bindingInfos = std::array {
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
		VK_CHECK(vkCreateDescriptorPool(Device, &poolInfo, nullptr, &m_BindlessPool));

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
			.descriptorPool = m_BindlessPool,
			.descriptorSetCount = 1,
			.pSetLayouts = &BindlessSetLayout
		};
		VK_CHECK(vkAllocateDescriptorSets(Device, &setInfo, &BindlessSet));

		VkPipelineLayoutCreateInfo pipelineInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 1,
			.pSetLayouts = &BindlessSetLayout,
			.pushConstantRangeCount = (uint32_t)PushConstantRanges.size(),
			.pPushConstantRanges = PushConstantRanges.data()
		};
		VK_CHECK(vkCreatePipelineLayout(Device, &pipelineInfo, nullptr, &BindlessPipelineLayout));
	}

	void VulkanContext::SetupFunctions()
	{
		#define GA_VULKAN_FUNCTION(name) {										\
				auto pfn = (PFN_##name)vkGetDeviceProcAddr(Device, #name);		\
				if (pfn) name = pfn;											\
		}
		#include "VulkanFunctions.inl"
	}

	void VulkanContext::Destroy()
	{
		if (m_ReferenceCount != 0) return;
		if (!m_ShutdownImplCalled) return;

		vkDeviceWaitIdle(Device);

		for (auto& deletionQueue : FrameDeletionQueues)
			deletionQueue.Flush();

		if (!ShaderObjectSupported)
		{
			size_t dataSize;
			vkGetPipelineCacheData(Device, m_PipelineCache, &dataSize, nullptr);
			std::vector<uint8_t> data(dataSize);
			vkGetPipelineCacheData(Device, m_PipelineCache, &dataSize, data.data());

			std::ofstream file("Assets/cache/pipelineCache.vulkan", std::ios::binary);
			file.write((const char*)data.data(), dataSize);

			vkDestroyPipelineCache(Device, m_PipelineCache, nullptr);
			for (auto& [key, pipeline] : m_GraphicsPipelines)
				vkDestroyPipeline(Device, pipeline, nullptr);
			for (auto& [key, pipeline] : m_ComputePipelines)
				vkDestroyPipeline(Device, pipeline, nullptr);
		}

		vkDestroyDescriptorPool(Device, m_BindlessPool, nullptr);
		vkDestroyDescriptorSetLayout(Device, BindlessSetLayout, nullptr);
		vkDestroyPipelineLayout(Device, BindlessPipelineLayout, nullptr);

		vmaDestroyAllocator(Allocator);

		vkDestroyDevice(Device, nullptr);
		vkDestroyDebugUtilsMessengerEXT(Instance, DebugMessenger, nullptr);
		vkDestroyInstance(Instance, nullptr);
	}

	VulkanContextReference::VulkanContextReference(const VulkanContextReference& other)
	{
		m_Context = other.m_Context;
		m_Context->m_ReferenceCount++;
	}

	VulkanContextReference::VulkanContextReference(VulkanContext* context)
		: m_Context(context)
	{
		m_Context->m_ReferenceCount++;
	}

	VulkanContextReference::~VulkanContextReference()
	{
		m_Context->m_ReferenceCount--;
		m_Context->Destroy();
	}

}