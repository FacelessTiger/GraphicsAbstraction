#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <GraphicsAbstraction/Core/Assert.h>
#include <GraphicsAbstraction/Renderer/Buffer.h>
#include <Renderer/Renderer.h>

namespace GraphicsAbstraction {

	Texture::Texture(const char* filename)
	{
		int width, height, channels;

		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load(filename, &width, &height, &channels, 4);
		GA_CORE_ASSERT(data);

		m_Image = Image::Create({ width, height }, ImageFormat::R8G8B8A8_UNORM, ImageUsage::Sampled | ImageUsage::TransferDst);
		auto stagingBuffer = Buffer::Create(width * height * 4, BufferUsage::TransferSrc, BufferFlags::Mapped);
		stagingBuffer->SetData(data);

		stbi_image_free(data);
		Renderer::CopyNextFrame(stagingBuffer, m_Image);
	}

}