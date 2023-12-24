#pragma once

#include <memory>
#include <glm/glm.hpp>

#include <GraphicsAbstraction/Core/Log.h>

namespace GraphicsAbstraction {

	class GraphicsContext;
	class Surface;
	class Image;

	class Swapchain
	{
	public:
		virtual ~Swapchain() = default;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void SetVsync(bool enabled) = 0;

		virtual std::shared_ptr<Image> GetCurrent() = 0;

		static std::shared_ptr<Swapchain> Create(const std::shared_ptr<Surface>& surface, const glm::vec2& size, bool enableVSync = true);
	};

}