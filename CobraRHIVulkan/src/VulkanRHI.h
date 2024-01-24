#pragma once

#include <GraphicsAbstraction/GraphicsAbstraction.h>
#include <GraphicsAbstraction/Renderer/Shared/PipelineKeys.h>

#include <InternalManagers/ResourceHandle.h>
#include <InternalManagers/PipelineManager.h>
#include <InternalManagers/RenderInfoManager.h>
#include <InternalManagers/DeletionQueue.h>
#include <InternalManagers/Utils.h>

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

#ifndef GA_DIST
	#define VK_CHECK(x)														\
			do                                                              \
			{                                                               \
				VkResult err = x;                                           \
				if (err)                                                    \
					assert(false && string_VkResult(err));					\
			} while (0)
#else
	#define VK_CHECK(x) x
#endif

namespace GraphicsAbstraction {

	inline bool g_ShaderObjectSupported = false;
	inline bool g_DynamicStateSupported = false;
	inline bool g_DynamicState2Supported = false;
	inline bool g_DynamicState3Supported = false;
	inline bool g_DynamicRenderingSupported = false;

	inline constexpr uint32_t STORAGE_BINDING = 0;
	inline constexpr uint32_t SAMPLER_BINDING = 1;
	inline constexpr uint32_t STORAGE_IMAGE_BINDING = 2;
	inline constexpr uint32_t SAMPLED_IMAGE_BINDING = 3;

	inline constexpr auto PUSH_CONSTANT_RANGES = std::array {
		VkPushConstantRange { VK_SHADER_STAGE_ALL, 0, 128 }
	};

	template<>
	struct Impl<GraphicsContext> : public RefCounted
	{
		static Ref<Impl<GraphicsContext>> Reference;
		VkInstance Instance;
		VkDebugUtilsMessengerEXT DebugMessenger;
		VkPhysicalDevice ChosenGPU;
		VkDevice Device;
		VmaAllocator Allocator;

		VkQueue GraphicsQueue;
		uint32_t GraphicsQueueFamily;

		VkDescriptorPool BindlessPool;
		VkDescriptorSetLayout BindlessSetLayout;
		VkDescriptorSet BindlessSet;
		VkPipelineLayout BindlessPipelineLayout;

		PipelineManager* PipelineManager;
		RenderInfoManager* RenderInfoManager;
		std::vector<DeletionQueue> FrameDeletionQueues;
		uint32_t FrameInFlight = 0;

#define GA_VULKAN_FUNCTION(name) PFN_##name name = (PFN_##name)+[]{ assert(false && "Function " #name " not loaded"); }
#include <InternalManagers/VulkanFunctions.inl>

		Impl(uint32_t frameInFlightCount);
		virtual ~Impl();

		void SetupInstance();
		void SetupPhysicalDevice();
		void SetupLogicalDevice();
		void SetupBindless();

		inline DeletionQueue& GetFrameDeletionQueue() { return FrameDeletionQueues[FrameInFlight]; }
	};

	template<>
	struct Impl<Queue>
	{
		VkQueue Queue;
		uint32_t QueueFamily;

		Impl<GraphicsContext>& Context;

		Impl(Impl<GraphicsContext>& context, VkQueue queue, uint32_t queueFamily);
	};

	template<>
	struct Impl<Image>
	{
		Utils::AllocatedImage Image;
		VkImageView View;
		VkImageLayout Layout;
		ImageFormat Format;
		ImageUsage Usage;

		uint32_t Width;
		uint32_t Height;

		ResourceHandle SampledHandle = { ResourceType::SampledImage };
		ResourceHandle StorageHandle = { ResourceType::StorageImage };

		Ref<Impl<GraphicsContext>> Context;
		bool ExternalAllocation = false;

		Impl(const glm::vec2& size, ImageFormat format, ImageUsage usage);
		Impl(VkImage image, VkImageView imageView, VkImageLayout imageLayout, ImageFormat imageFormat, ImageUsage usage, uint32_t width, uint32_t height);

		void TransitionLayout(VkCommandBuffer cmd, VkImageLayout newLayout);

		void ConstructImage();
		void Destroy();
		void UpdateDescriptor();
	};

	template<>
	struct Impl<Buffer>
	{
		uint32_t Size;
		Utils::AllocatedBuffer Buffer;

		ResourceHandle Handle = { ResourceType::StorageBuffer };
		Ref<Impl<GraphicsContext>> Context;

		Impl(uint32_t size, BufferUsage usage, BufferFlags flags);
		void UpdateDescriptor();
	};

	template<>
	struct Impl<Swapchain>
	{
		VkSwapchainKHR Swapchain;
		VkFormat ImageFormat;
		std::vector<Ref<Image>> Images;

		std::vector<VkSemaphore> Semaphores;
		uint32_t SemaphoreIndex = 0;
		uint32_t ImageIndex = 0;

		uint32_t Width, Height;
		bool Dirty = false;

		Ref<Impl<GraphicsContext>> Context;
		VkSurfaceKHR Surface;
		bool EnableVsync;

		VkSurfaceFormatKHR ChosenSufaceFormat;
		VkPresentModeKHR VsyncOnPresent;
		VkPresentModeKHR VsyncOffPresent;

		Impl(const Ref<Window>& window, const glm::vec2& size, bool enableVSync);
		void Recreate();

		void CreateSurface(const Ref<Window>& window);
		void CreateSwapchain(bool firstCreation);

		VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseVsyncOffPresent(const std::vector<VkPresentModeKHR>& presentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	};

	template<>
	struct Impl<Fence>
	{
		VkSemaphore TimelineSemaphore;
		uint64_t Value = 0;

		Ref<Impl<GraphicsContext>> Context;

		Impl();
	};

	template<>
	struct Impl<CommandList>
	{
		VkCommandBuffer CommandBuffer;
		Ref<Impl<GraphicsContext>> Context;

		GraphicsPipelineKey GraphicsPipelineKey;
		ComputePipelineKey ComputePipelineKey;
		bool GraphicsPipelineStateChanged = false;
		bool ComputePipelineStateChanged = false;

		bool DefaultDynamicStateSet = false;
		bool FillModeSet = false;
		bool DepthEnableSet = false;
		bool ColorBlendSet = false;

		Impl(VkCommandBuffer buffer);

		void SetColorBlend(bool enabled, Blend srcBlend, Blend dstBlend, BlendOp blendOp, Blend srcBlendAlpha, Blend dstBlendAlpha, BlendOp blendAlpha);
		void SetDynamicState();
	};

	template<>
	struct Impl<Shader>
	{
		VkShaderStageFlagBits Stage;
		uint32_t ID;

		VkShaderEXT ShaderObject;
		VkShaderModule Module;
		VkPipelineShaderStageCreateInfo StageInfo;

		Ref<Impl<GraphicsContext>> Context;
		std::string Path;

		Impl(const std::vector<uint32_t>& data, ShaderStage stage);
		Impl(const std::string& path, ShaderStage stage, std::vector<uint32_t>* compiledData);
		static Impl<Shader>* GetShaderByID(uint32_t id);

		void CreatePipelineShaderStage(const std::vector<uint32_t>& data);
	};

	template<>
	struct Impl<CommandAllocator>
	{
		VkCommandPool CommandPool;
		VkCommandBuffer MainCommandBuffer;

		Ref<Impl<GraphicsContext>> Context;

		Impl(const Ref<Queue>& queue);
	};

	template<>
	struct Impl<Sampler>
	{
		VkSampler Sampler;
		ResourceHandle Handle = { ResourceType::Sampler };

		Ref<Impl<GraphicsContext>> Context;

		Impl(Filter min, Filter mag);
	};

}