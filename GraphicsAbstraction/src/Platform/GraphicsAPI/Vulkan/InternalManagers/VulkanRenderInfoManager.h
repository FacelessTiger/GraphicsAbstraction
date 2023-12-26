#pragma once

#include <Platform/GraphicsAPI/Vulkan/InternalManagers/VulkanRenderInfoKeys.h>

#include <vulkan/vulkan.h>

namespace GraphicsAbstraction {

	class VulkanContext;

	struct VulkanRenderInfo
	{
		VkRenderPass renderpass;
		VkFramebuffer framebuffer;
	};

	class VulkanRenderInfoManager
	{
	public:
		VulkanRenderInfoManager(VulkanContext& context);
		~VulkanRenderInfoManager();

		VulkanRenderInfo GetRenderInfo(const VulkanRenderInfoKey& key);
		void ClearFramebufferCache();
	private:
		std::unordered_map<VulkanRenderInfoKey, VkRenderPass> m_Renderpasses;
		std::unordered_map<VulkanRenderInfoKey, VkFramebuffer> m_Framebuffers;

		VulkanContext& m_Context;
	};

}