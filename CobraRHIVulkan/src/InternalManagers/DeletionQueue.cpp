#include "DeletionQueue.h"

#include <VulkanRHI.h>

namespace GraphicsAbstraction {

	DeletionQueue::DeletionQueue(Impl<GraphicsContext>& context)
		: m_Context(context)
	{ }

	DeletionQueue::~DeletionQueue()
	{
		
	}

	void DeletionQueue::Flush()
	{
		for (auto commandPool : m_CommandPools)
			vkDestroyCommandPool(m_Context.Device, commandPool, nullptr);
		m_CommandPools.clear();

		for (auto semaphore : m_Semaphores)
			vkDestroySemaphore(m_Context.Device, semaphore, nullptr);
		m_Semaphores.clear();

		for (auto imageView : m_ImageViews)
			vkDestroyImageView(m_Context.Device, imageView, nullptr);
		m_ImageViews.clear();

		for (auto swapchain : m_Swapchains)
			vkDestroySwapchainKHR(m_Context.Device, swapchain, nullptr);
		m_Swapchains.clear();

		for (auto& image : m_Images)
			vmaDestroyImage(m_Context.Allocator, image.Image, image.Allocation);
		m_Images.clear();

		for (auto& buffer : m_Buffers)
			vmaDestroyBuffer(m_Context.Allocator, buffer.Buffer, buffer.Allocation);
		m_Buffers.clear();

		for (auto& module : m_ShaderModules)
			vkDestroyShaderModule(m_Context.Device, module, nullptr);
		m_ShaderModules.clear();

		for (auto& shader : m_Shaders)
			m_Context.vkDestroyShaderEXT(m_Context.Device, shader, nullptr);
		m_Shaders.clear();

		for (auto& sampler : m_Samplers)
			vkDestroySampler(m_Context.Device, sampler, nullptr);
		m_Samplers.clear();

		for (auto& surface : m_Surfaces)
			vkDestroySurfaceKHR(m_Context.Instance, surface, nullptr);
		m_Surfaces.clear();

		for (auto& framebuffer : m_Framebuffers)
			vkDestroyFramebuffer(m_Context.Device, framebuffer, nullptr);
		m_Framebuffers.clear();
	}

}