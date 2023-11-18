#pragma once

#include <GraphicsAbstraction/Core/Core.h>
#include <GraphicsAbstraction/Renderer/Swapchain.h>

#include <glm/glm.hpp>

#include <vector>

namespace GraphicsAbstraction {

	class CommandBuffer;

	class Renderpass
	{
	public:
		enum class LoadOperation
		{
			None = 0, // Don't care
			Clear
		};

		enum class StoreOperation
		{
			None = 0, // Don't care
			Store
		};

		enum class ImageLayout
		{
			None = 0, // Don't care
			ColorAttachmentOptimal, // Layout optimal to be written into by rendering commands
			PresentSource // Layout that allows displaying on the screen
		};

		enum class PipelineBindpoint
		{
			Graphics = 0
		};

		struct Attachment
		{
			Renderpass::LoadOperation LoadOperation = Renderpass::LoadOperation::None;
			Renderpass::StoreOperation StoreOperation = Renderpass::StoreOperation::None;
			Renderpass::LoadOperation StencilLoadOperation = Renderpass::LoadOperation::None;
			Renderpass::StoreOperation StencilStoreOperation = Renderpass::StoreOperation::None;

			ImageLayout InitialImageLayout = ImageLayout::None;
			ImageLayout FinalImageLayout = ImageLayout::None;
		};

		struct AttachmentReference
		{
			uint32_t AttachmentIndex = -1;
			ImageLayout ImageLayout = ImageLayout::None;
		};

		struct SubpassSpecification
		{
			PipelineBindpoint Bindpoint = PipelineBindpoint::Graphics;
			std::vector<AttachmentReference> ColorAttachments;
		};

		struct Specification
		{
			std::vector<Attachment> Attachments;
			std::vector<SubpassSpecification> Subpasses;
		};
	public:
		virtual ~Renderpass() = default;

		virtual void Begin(std::shared_ptr<Swapchain> swapchain, std::shared_ptr<CommandBuffer> cmd, const glm::vec4& clearColor, uint32_t swapchainImageIndex) const = 0;
		virtual void End(std::shared_ptr<CommandBuffer> cmd) const = 0;

		virtual void Recreate(std::shared_ptr<Swapchain> swapchain) = 0;

		static std::shared_ptr<Renderpass> Create(const Specification& spec, std::shared_ptr<GraphicsContext> context, std::shared_ptr<Swapchain> swapchain);
	};

}