#pragma once

#include <GraphicsAbstraction/Renderer/Image.h>

namespace GraphicsAbstraction {

	class D3D12Image : public Image
	{
	public:
		D3D12Image(const glm::vec2& size, ImageFormat format, ImageUsage usage) { }

		void CopyTo(const Ref<CommandBuffer>& cmd, const Ref<Image>& other) override { }
		void Resize(const glm::vec2& size) override { }

		uint32_t GetHandle() const override { return 0; }
		glm::vec2 GetSize() const override { return glm::vec2(0.0f); }
	};

}