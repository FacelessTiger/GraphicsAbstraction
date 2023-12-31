#pragma once

#include <GraphicsAbstraction/Renderer/Swapchain.h>
#include <GraphicsAbstraction/Renderer/Surface.h>
#include <GraphicsAbstraction/Renderer/Image.h>

namespace GraphicsAbstraction {

	class D3D12Swapchain : public Swapchain
	{
	public:
		D3D12Swapchain(Ref<Surface>& surface, const glm::vec2& size, bool enableVSync) { }

		void Resize(uint32_t width, uint32_t height) override { }
		void SetVsync(bool enabled) override { }

		Ref<Image> GetCurrent() override { return nullptr; }
	};

}