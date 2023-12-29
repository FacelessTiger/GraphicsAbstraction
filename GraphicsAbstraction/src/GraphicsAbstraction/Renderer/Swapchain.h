#pragma once

#include <memory>
#include <glm/glm.hpp>

#include <GraphicsAbstraction/Core/Log.h>
#include <GraphicsAbstraction/Core/Core.h>

namespace GraphicsAbstraction {

	class GraphicsContext;
	class Surface;
	class Image;

	class Swapchain : public RefCounted
	{
	public:
		virtual ~Swapchain() = default;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void SetVsync(bool enabled) = 0;

		virtual Ref<Image> GetCurrent() = 0;

		static Ref<Swapchain> Create(Ref<Surface>& surface, const glm::vec2& size, bool enableVSync = true);
	};

}