#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <InternalManagers/Utils.h>

#include <vector>
#include <list>
#include <memory>

namespace GraphicsAbstraction {

	class DeletionQueue
	{
	public:
		DeletionQueue(Impl<GraphicsContext>& context);
		virtual ~DeletionQueue();

		inline void Push(VkCommandPool commandPool) { m_CommandPools.push_back(commandPool); }
		inline void Push(VkSemaphore semaphore) { m_Semaphores.push_back(semaphore); }
		inline void Push(VkImageView imageView) { m_ImageViews.push_back(imageView); }
		inline void Push(VkSwapchainKHR swapchain) { m_Swapchains.push_back(swapchain); }
		inline void Push(Utils::AllocatedImage image) { m_Images.push_back(image); }
		inline void Push(Utils::AllocatedBuffer buffer) { m_Buffers.push_back(buffer); }
		inline void Push(VkShaderModule module) { m_ShaderModules.push_back(module); }
		inline void Push(VkShaderEXT shader) { m_Shaders.push_back(shader); }
		inline void Push(VkSampler sampler) { m_Samplers.push_back(sampler); }
		inline void Push(VkSurfaceKHR surface) { m_Surfaces.push_back(surface); }
		inline void Push(VkFramebuffer framebuffer) { m_Framebuffers.push_back(framebuffer); }

		void Flush();
	private:
		Impl<GraphicsContext>& m_Context;

		std::vector<VkCommandPool> m_CommandPools;
		std::vector<VkSemaphore> m_Semaphores;
		std::vector<VkImageView> m_ImageViews;
		std::vector<VkSwapchainKHR> m_Swapchains;
		std::vector<Utils::AllocatedImage> m_Images;
		std::vector<Utils::AllocatedBuffer> m_Buffers;
		std::vector<VkShaderModule> m_ShaderModules;
		std::vector<VkShaderEXT> m_Shaders;
		std::vector<VkSampler> m_Samplers;
		std::vector<VkSurfaceKHR> m_Surfaces;
		std::vector<VkFramebuffer> m_Framebuffers;
	};

}