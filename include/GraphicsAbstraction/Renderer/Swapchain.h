#pragma once

#include <memory>
#include <glm/glm.hpp>

#include <GraphicsAbstraction/Core/Core.h>

namespace GraphicsAbstraction {

	struct GraphicsContext;
	struct Window;
	struct Image;

	struct GA_DLL_LINK Swapchain : public RefCounted
	{
		void Resize(uint32_t width, uint32_t height);
		void SetVsync(bool enabled);

		Ref<Image> GetCurrent();

		GA_RHI_TEMPLATE(Swapchain, const Ref<Window>& window, const glm::vec2& size, bool enableVSync = true)
	};

}