#pragma once

#include <GraphicsAbstraction/Renderer/GraphicsContext.h>
#include <Platform/GraphicsAPI/Vulkan/InternalManagers/VulkanDeletionQueue.h>
#include <Platform/GraphicsAPI/Vulkan/InternalManagers/VulkanPipelineManager.h>
#include <Platform/GraphicsAPI/Vulkan/InternalManagers/VulkanRenderInfoManager.h>
#include <Platform/GraphicsAPI/Shared/PipelineKeys.h>

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vk_mem_alloc.h>

#ifndef GA_DIST
	#define VK_CHECK(x)                                                 \
		do                                                              \
		{                                                               \
			VkResult err = x;                                           \
			if (err)                                                    \
				GA_CORE_ASSERT(false, string_VkResult(err))				\
		} while (0)
#else
	#define VK_CHECK(x) x
#endif

namespace GraphicsAbstraction {

	class VulkanShader;
	class VulkanContext;

	class VulkanContext : public GraphicsContext
	{
	public:
		VkInstance Instance;
		VkDebugUtilsMessengerEXT DebugMessenger;
		VkPhysicalDevice ChosenGPU;
		VkDevice Device;
		VmaAllocator Allocator;

		VkQueue GraphicsQueue;
		uint32_t GraphicsQueueFamily;

		VkDescriptorSetLayout BindlessSetLayout;
		VkDescriptorSet BindlessSet;
		VkPipelineLayout BindlessPipelineLayout;

		VulkanPipelineManager* PipelineManager;
		VulkanRenderInfoManager* RenderInfoManager;
		std::vector<VulkanDeletionQueue> FrameDeletionQueues;
		uint32_t FrameInFlight = 0;

		static constexpr uint32_t STORAGE_BINDING = 0;
		static constexpr uint32_t SAMPLER_BINDING = 1;
		static constexpr uint32_t STORAGE_IMAGE_BINDING = 2;
		static constexpr uint32_t SAMPLED_IMAGE_BINDING = 3;

		static constexpr auto PushConstantRanges = std::array {
			VkPushConstantRange { VK_SHADER_STAGE_ALL, 0, 128 }
		};

		bool ShaderObjectSupported = false;
		bool DynamicStateSupported = false;
		bool DynamicState2Supported = false;
		bool DynamicState3Supported = false;
		bool DynamicRenderingSupported = false;

		#define GA_VULKAN_FUNCTION(name) PFN_##name name = (PFN_##name)+[]{ GA_CORE_ASSERT(false, "Function " #name " not loaded"); }
		#include "../InternalManagers/VulkanFunctions.inl"
	public:
		VulkanContext(uint32_t frameInFlightCount);
		~VulkanContext();

		Ref<Queue> GetQueueImpl(QueueType type) override;
		inline void SetFrameInFlightImpl(uint32_t fif) override { FrameInFlight = fif; FrameDeletionQueues[FrameInFlight].Flush(); }

		inline VulkanDeletionQueue& GetFrameDeletionQueue() { return FrameDeletionQueues[FrameInFlight]; }
		inline static Ref<VulkanContext> GetReference() { return { s_Instance, (VulkanContext*)s_Instance.Get() }; }
	private:
		void SetupInstance();
		void SetupPhysicalDevice();
		void SetupLogicalDevice();
		void SetupBindless();
	private:
#ifndef GA_DIST
		bool m_UseValidationLayers = true;
#else
		bool m_UseValidationLayers = false;
#endif

		VkDescriptorPool m_BindlessPool;
	};

}