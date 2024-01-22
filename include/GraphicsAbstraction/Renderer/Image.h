#pragma once

#include <memory>
#include <glm/glm.hpp>

#include <GraphicsAbstraction/Core/Core.h>

namespace GraphicsAbstraction {

	struct GraphicsContext;
	struct CommandList;

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

	struct GA_DLL_LINK Image : public RefCounted
	{
		void Resize(const glm::vec2& size);

		uint32_t GetSampledHandle() const;
		uint32_t GetStorageHandle() const;
		uint32_t GetWidth() const;
		uint32_t GetHeight() const;
		glm::vec2 GetSize() const;

		GA_RHI_TEMPLATE(Image, const glm::vec2& size, ImageFormat format, ImageUsage usage)
	};

}