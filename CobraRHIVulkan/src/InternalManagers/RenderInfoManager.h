#pragma once

#include <GraphicsAbstraction/Renderer/GraphicsContext.h>
#include <InternalManagers/RenderInfoKeys.h>

#include <vulkan/vulkan.h>

namespace GraphicsAbstraction {

	struct RenderInfo
	{
		VkRenderPass renderpass;
		VkFramebuffer framebuffer;
	};

	class RenderInfoManager
	{
	public:
		RenderInfoManager(Impl<GraphicsContext>& context);
		~RenderInfoManager();

		RenderInfo GetRenderInfo(const RenderInfoKey& key);
		void ClearFramebufferCache();
	private:
		std::unordered_map<RenderInfoKey, VkRenderPass> m_Renderpasses;
		std::unordered_map<RenderInfoKey, VkFramebuffer> m_Framebuffers;

		Impl<GraphicsContext>& m_Context;
	};

}