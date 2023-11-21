#pragma once

#include <GraphicsAbstraction/Core/Core.h>

#include <memory>
#include <glm/glm.hpp>

namespace GraphicsAbstraction {

	class Window;
	class GraphicsContext;
	class Image;

	class Swapchain
	{
	public:
		virtual ~Swapchain() = default;

		virtual uint32_t AcquireNextImage() const = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual const glm::vec2& GetSize() const = 0;

		virtual const std::vector<std::shared_ptr<Image>>& GetImages() const = 0;

		static std::shared_ptr<Swapchain> Create(std::shared_ptr<Window> window, std::shared_ptr<GraphicsContext> context);
	};

}