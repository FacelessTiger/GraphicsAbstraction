#pragma once

#include <memory>
#include <glm/glm.hpp>

#include <GraphicsAbstraction/Core/Core.h>
#include <GraphicsAbstraction/Core/Log.h>

namespace GraphicsAbstraction {

	class GraphicsContext;
	class CommandList;

	enum class ImageFormat
	{
		Unknown = 0,
		R16G16B16A16_SFLOAT,
		R8G8B8A8_UNORM,
		B8G8R8A8_SRGB,
		D32_SFLOAT
	};

	enum class ImageUsage : uint32_t
	{
		None = 0,
		ColorAttachment = 1,
		DepthStencilAttachment = 2,
		TransferSrc = 4,
		TransferDst = 8,
		Storage = 16,
		Sampled = 32
	};

	inline ImageUsage operator|(ImageUsage a, ImageUsage b) { return (ImageUsage)((int)a | (int)b); };
	inline ImageUsage& operator|=(ImageUsage& a, ImageUsage b) { return a = a | b; };
	inline bool operator&(ImageUsage a, ImageUsage b) { return (int)a & (int)b; };

	class Image : public RefCounted
	{
	public:
		virtual ~Image() = default;

		virtual void Resize(const glm::vec2& size) = 0;

		virtual uint32_t GetSampledHandle() const = 0;
		virtual uint32_t GetStorageHandle() const = 0;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual glm::vec2 GetSize() const = 0;

		static Ref<Image> Create(const glm::vec2& size, ImageFormat format, ImageUsage usage);
	};

}