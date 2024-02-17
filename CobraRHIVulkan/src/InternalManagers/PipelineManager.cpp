#include "PipelineManager.h"

#include <VulkanRHI.h>
#include <fstream>

namespace GraphicsAbstraction {

	PipelineManager::PipelineManager(Impl<GraphicsContext>& context)
		: m_Context(context)
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

		VK_CHECK(vkCreatePipelineCache(m_Context.Device, &cacheInfo, nullptr, &m_PipelineCache));
	}

	PipelineManager::~PipelineManager()
	{
		size_t dataSize;
		vkGetPipelineCacheData(m_Context.Device, m_PipelineCache, &dataSize, nullptr);
		std::vector<uint8_t> data(dataSize);
		vkGetPipelineCacheData(m_Context.Device, m_PipelineCache, &dataSize, data.data());

		std::ofstream file("Assets/cache/pipelineCache.vulkan", std::ios::binary);
		file.write((const char*)data.data(), dataSize);

		vkDestroyPipelineCache(m_Context.Device, m_PipelineCache, nullptr);
		for (auto& [key, pipeline] : m_GraphicsPipelines)
			vkDestroyPipeline(m_Context.Device, pipeline, nullptr);
		for (auto& [key, pipeline] : m_ComputePipelines)
			vkDestroyPipeline(m_Context.Device, pipeline, nullptr);
	}

	VkPipeline PipelineManager::GetGraphicsPipeline(const GraphicsPipelineKey& key)
	{
		if (m_GraphicsPipelines.find(key) != m_GraphicsPipelines.end()) return m_GraphicsPipelines[key];

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		shaderStages.reserve(key.Shaders.size());
		for (uint32_t id : key.Shaders)
		{
			if (id) shaderStages.push_back(Impl<Shader>::GetShaderByID(id)->StageInfo);
		}

		std::vector<VkFormat> colorAttachmentFormats;
		std::vector<VkPipelineColorBlendAttachmentState> colorAttachmentBlends;
		colorAttachmentFormats.reserve(key.ColorAttachments.size());

		for (const ColorAttachment& attachment : key.ColorAttachments)
		{
			if (attachment.Format == ImageFormat::Unknown) break;

			colorAttachmentFormats.push_back(Utils::GAImageFormatToVulkan(attachment.Format));
			colorAttachmentBlends.push_back({
				.blendEnable = attachment.BlendInfo.BlendEnable,
				.srcColorBlendFactor = Utils::GABlendToVulkan(attachment.BlendInfo.SrcBlend),
				.dstColorBlendFactor = Utils::GABlendToVulkan(attachment.BlendInfo.DstBlend),
				.colorBlendOp = Utils::GABlendOpToVulkan(attachment.BlendInfo.BlendOp),
				.srcAlphaBlendFactor = Utils::GABlendToVulkan(attachment.BlendInfo.SrcBlendAlpha),
				.dstAlphaBlendFactor = Utils::GABlendToVulkan(attachment.BlendInfo.DstBlendAlpha),
				.alphaBlendOp = Utils::GABlendOpToVulkan(attachment.BlendInfo.BlendOpAlpha),
				.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
			});
		}

		VkPipelineColorBlendStateCreateInfo colorBlendInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = (uint32_t)colorAttachmentBlends.size(),
			.pAttachments = colorAttachmentBlends.data()
		};

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		VkPipelineViewportStateCreateInfo viewportStateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };

		VkPipelineRasterizationStateCreateInfo rasterizationInfo = { 
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, 
			.rasterizerDiscardEnable = false,
			.polygonMode = Utils::GAFillModeToVulkan(key.FillMode),
			.cullMode = VK_CULL_MODE_NONE,
			.frontFace = VK_FRONT_FACE_CLOCKWISE,
			.depthBiasEnable = false
		};

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = false
		};

		VkPipelineMultisampleStateCreateInfo multisampleInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.minSampleShading = 1.0f,
			.pSampleMask = nullptr,
			.alphaToCoverageEnable = VK_FALSE,
			.alphaToOneEnable = VK_FALSE
		};

		VkPipelineDepthStencilStateCreateInfo depthInfo = { 
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable = key.DepthTestEnable,
			.depthWriteEnable = key.DepthWriteEnable,
			.depthCompareOp = Utils::GACompareOpToVulkan(key.DepthCompareOp),
			.depthBoundsTestEnable = false,
			.stencilTestEnable = false
		};

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_LINE_WIDTH, VK_DYNAMIC_STATE_BLEND_CONSTANTS, VK_DYNAMIC_STATE_DEPTH_BOUNDS, VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
			VK_DYNAMIC_STATE_STENCIL_WRITE_MASK, VK_DYNAMIC_STATE_STENCIL_REFERENCE,
		};

		if (g_DynamicStateSupported) dynamicStates.insert(dynamicStates.end(), {
			VK_DYNAMIC_STATE_CULL_MODE, VK_DYNAMIC_STATE_FRONT_FACE, VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY, VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT, 
			VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT, VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE, VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE, 
			VK_DYNAMIC_STATE_DEPTH_COMPARE_OP, VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE, VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE, VK_DYNAMIC_STATE_STENCIL_OP
		});
		else
		{
			dynamicStates.insert(dynamicStates.end(), { // you can't have both with_count and no with count version activated, so we need an else
				VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
			});

			viewportStateInfo.viewportCount = 1;
			viewportStateInfo.scissorCount = 1;
		}

		if (g_DynamicState2Supported) dynamicStates.insert(dynamicStates.end(), {
			VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE, VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE, VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE
		});

		if (g_DynamicState3Supported) dynamicStates.insert(dynamicStates.end(), {
			VK_DYNAMIC_STATE_POLYGON_MODE_EXT, VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT, VK_DYNAMIC_STATE_SAMPLE_MASK_EXT, VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT,
			VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT, VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT , VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT
		});

		VkPipelineDynamicStateCreateInfo dynamicInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = (uint32_t)dynamicStates.size(),
			.pDynamicStates = dynamicStates.data()
		};

		VkPipelineRenderingCreateInfo renderingInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			.colorAttachmentCount = (uint32_t)colorAttachmentFormats.size(),
			.pColorAttachmentFormats = colorAttachmentFormats.data(),
			.depthAttachmentFormat = Utils::GAImageFormatToVulkan(key.DepthAttachment)
		};

		VkGraphicsPipelineCreateInfo pipelineInfo = {
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
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
			.layout = m_Context.BindlessPipelineLayout
		};
		if (g_DynamicRenderingSupported) pipelineInfo.pNext = &renderingInfo;
		else pipelineInfo.renderPass = (VkRenderPass)key.Renderpass;

		VkPipeline pipeline;
		VK_CHECK(vkCreateGraphicsPipelines(m_Context.Device, m_PipelineCache, 1, &pipelineInfo, nullptr, &pipeline));
		m_GraphicsPipelines[key] = pipeline;

		return pipeline;
	}

	VkPipeline PipelineManager::GetComputePipeline(const ComputePipelineKey& key)
	{
		if (m_ComputePipelines.find(key) != m_ComputePipelines.end()) return m_ComputePipelines[key];

		VkComputePipelineCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.stage = Impl<Shader>::GetShaderByID(key.Shader)->StageInfo,
			.layout = m_Context.BindlessPipelineLayout
		};

		VkPipeline pipeline;
		VK_CHECK(vkCreateComputePipelines(m_Context.Device, m_PipelineCache, 1, &createInfo, nullptr, &pipeline));
		m_ComputePipelines[key] = pipeline;

		return pipeline;
	}

}