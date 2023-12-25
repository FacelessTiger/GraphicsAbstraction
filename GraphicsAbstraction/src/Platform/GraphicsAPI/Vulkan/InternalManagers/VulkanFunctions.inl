#ifndef GA_VULKAN_FUNCTION
	#define GA_VULKAN_FUNCTION(...)
#endif

// Debug setup
GA_VULKAN_FUNCTION(vkCreateDebugUtilsMessengerEXT);
GA_VULKAN_FUNCTION(vkDestroyDebugUtilsMessengerEXT);

// Shader objects
GA_VULKAN_FUNCTION(vkCmdBindShadersEXT);
GA_VULKAN_FUNCTION(vkCreateShadersEXT);
GA_VULKAN_FUNCTION(vkDestroyShaderEXT);
GA_VULKAN_FUNCTION(vkCmdSetVertexInputEXT);

// Dynamic state
GA_VULKAN_FUNCTION(vkCmdSetCullModeEXT);
GA_VULKAN_FUNCTION(vkCmdSetFrontFaceEXT);
GA_VULKAN_FUNCTION(vkCmdSetPrimitiveTopologyEXT);
GA_VULKAN_FUNCTION(vkCmdSetViewportWithCountEXT);
GA_VULKAN_FUNCTION(vkCmdSetScissorWithCountEXT);
GA_VULKAN_FUNCTION(vkCmdSetDepthTestEnableEXT);
GA_VULKAN_FUNCTION(vkCmdSetDepthWriteEnableEXT);
GA_VULKAN_FUNCTION(vkCmdSetDepthCompareOpEXT);
GA_VULKAN_FUNCTION(vkCmdSetDepthBoundsTestEnableEXT);
GA_VULKAN_FUNCTION(vkCmdSetStencilTestEnableEXT);
GA_VULKAN_FUNCTION(vkCmdSetStencilOpEXT);

// Dynamic state 2
GA_VULKAN_FUNCTION(vkCmdSetRasterizerDiscardEnableEXT);
GA_VULKAN_FUNCTION(vkCmdSetDepthBiasEnableEXT);
GA_VULKAN_FUNCTION(vkCmdSetPrimitiveRestartEnableEXT);

// Dynamic state 3
GA_VULKAN_FUNCTION(vkCmdSetPolygonModeEXT);
GA_VULKAN_FUNCTION(vkCmdSetRasterizationSamplesEXT);
GA_VULKAN_FUNCTION(vkCmdSetSampleMaskEXT);
GA_VULKAN_FUNCTION(vkCmdSetAlphaToCoverageEnableEXT);
GA_VULKAN_FUNCTION(vkCmdSetColorBlendEnableEXT);
GA_VULKAN_FUNCTION(vkCmdSetColorBlendEquationEXT);
GA_VULKAN_FUNCTION(vkCmdSetColorWriteMaskEXT);

// Dynamic rendering
GA_VULKAN_FUNCTION(vkCmdBeginRenderingKHR);
GA_VULKAN_FUNCTION(vkCmdEndRenderingKHR);

#undef GA_VULKAN_FUNCTION