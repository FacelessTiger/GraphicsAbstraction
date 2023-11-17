#pragma once

#ifndef GA_DIST
	#include <tracy/Tracy.hpp>

	#ifdef GA_RENDERER_VULKAN
		#include <vulkan/vulkan.h>
		#include <tracy/TracyVulkan.hpp>

		#include <Platform/GraphicsAPI/Vulkan/VulkanContext.h>
		#include <Platform/GraphicsAPI/Vulkan/VulkanCommandBuffer.h>
	#endif
#endif

#ifndef GA_DIST
	#define GA_PROFILE_SCOPE() ZoneScoped
	#define GA_FRAME_MARK()	FrameMark

	#ifdef GA_RENDERER_VULKAN
		using GPUProfilerContext = tracy::VkCtx;

		#define GA_GPU_PROFILER_CONTEXT(graphicsContext, commandBuffer)													\
				[&]()																									\
				{																										\
					auto vulkanContext = std::dynamic_pointer_cast<VulkanContext>(graphicsContext);						\
					auto vulkanCmdBuf = std::dynamic_pointer_cast<VulkanCommandBuffer>(commandBuffer);					\
																														\
					return TracyVkContextCalibrated(vulkanContext->GetPhysicalDevice(),									\
													vulkanContext->GetLogicalDevice(),									\
													vulkanContext->GetGraphicsQeue(),									\
													vulkanCmdBuf->GetInternal(),										\
													vulkanContext->GetPhysicalDeviceCalibrateableTimeDomainsEXT(),		\
													vulkanContext->GetCalibratedTimestampsEXT());																																		\
				}()
		#define GA_PROFILE_GPU_SCOPE(ctx, cmdBuf, name) TracyVkZone(ctx, std::dynamic_pointer_cast<VulkanCommandBuffer>(cmdBuf)->GetInternal(), name)
		#define GA_PROFILE_GPU_COLLECT(ctx, cmdBuf) TracyVkCollect(ctx, std::dynamic_pointer_cast<VulkanCommandBuffer>(cmdBuf)->GetInternal())
		#define GA_GPU_PROFILER_DESTROY(ctx) TracyVkDestroy(ctx)
	#endif
#else
	#define GA_PROFILE_SCOPE()
	#define GA_FRAME_MARK()

	using GPUProfilerContext = void;

	#define GA_GPU_PROFILER_CONTEXT(graphicsContext, commandBuffer) nullptr
	#define GA_PROFILE_GPU_SCOPE(ctx, cmdBuf, name)
	#define GA_PROFILE_GPU_COLLECT(ctx, cmdBuf)
	#define GA_GPU_PROFILER_DESTROY(ctx)
#endif