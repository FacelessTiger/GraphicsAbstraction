#pragma once

#include <GraphicsAbstraction/Core/Core.h>
#include <GraphicsAbstraction/Renderer/Swapchain.h>

#include <glm/glm.hpp>

#include <vector>

namespace GraphicsAbstraction {

	class CommandBuffer;
	class Image;

	struct ClearValue
	{
		glm::vec4 ClearColor = glm::vec4(0.0f);
		float Depth = 0.0f;
		uint32_t Stencil = 0;

		ClearValue(const glm::vec4& clearColor)
			: ClearColor(clearColor)
		{ }
		ClearValue(float depth, uint32_t stencil)
			: Depth(depth), Stencil(stencil)
		{ }
	};

	class Renderpass
	{
	public:
		struct Attachment
		{
			std::vector<std::shared_ptr<Image>> Images;

			Attachment(const std::vector<std::shared_ptr<Image>>& images)
				: Images(images)
			{ }
			Attachment(std::shared_ptr<Image> image)
			{ Images.push_back(image); }
		};

		struct Specification
		{
			std::vector<Attachment> ColorInputs;
			std::vector<Attachment> ColorOutputs;
			std::vector<Attachment> DepthStencilInput;
			std::vector<Attachment> DepthStencilOutput;

			uint32_t FramebufferCount;
			glm::vec2 Size;
		};
	public:
		virtual ~Renderpass() = default;

		virtual void Begin(const glm::vec2& size, std::shared_ptr<CommandBuffer> cmd, const std::vector<ClearValue>& clearValues, uint32_t swapchainImageIndex) const = 0;
		virtual void End(std::shared_ptr<CommandBuffer> cmd) const = 0;

		virtual void Recreate(const Specification& spec) = 0;

		static std::shared_ptr<Renderpass> Create(std::shared_ptr<GraphicsContext> context, const Specification& spec);
	};

}