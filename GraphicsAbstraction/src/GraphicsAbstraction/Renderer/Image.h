#pragma once

#include <memory>
#include <glm/glm.hpp>

#include <GraphicsAbstraction/Core/Core.h>
#include <GraphicsAbstraction/Core/Log.h>

namespace GraphicsAbstraction {

	class GraphicsContext;
	class CommandBuffer;

	enum class ImageFormat
	{
		R16G16B16A16_SFLOAT,
		R8G8B8A8_UNORM,
		D32_SFLOAT
	};

	enum class ImageUsage : uint32_t
	{
		ColorAttachment = 1,
		DepthStencilAttachment = 2,
		TransferSrc = 4,
		TransferDst = 8,
		Storage = 16,
		Sampled = 32
	};

	inline ImageUsage operator|(ImageUsage a, ImageUsage b) { return (ImageUsage)((int)a | (int)b); };
	inline bool operator&(ImageUsage a, ImageUsage b) { return (int)a & (int)b; };

	class Image : public RefCounted
	{
	public:
		virtual ~Image() = default;

		virtual void CopyTo(const Ref<CommandBuffer>& cmd, const Ref<Image>& other) = 0;
		virtual void Resize(const glm::vec2& size) = 0;

		virtual uint32_t GetHandle() const = 0;
		virtual glm::vec2 GetSize() const = 0;

		static Ref<Image> Create(const glm::vec2& size, ImageFormat format, ImageUsage usage);
	};

}