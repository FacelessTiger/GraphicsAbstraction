#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <GraphicsAbstraction/Renderer/Buffer.h>
#include <Core/Assert.h>
#include <Renderer/Renderer.h>

namespace GraphicsAbstraction {

	Texture::Texture(const char* filename)
	{
		int width, height, channels;

		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load(filename, &width, &height, &channels, 4);
		GA_CORE_ASSERT(data);

		CreateVulkanImage(data, width, height);
		stbi_image_free(data);
	}

	Texture::Texture(uint8_t* data, uint32_t size)
	{
		int width, height, channels;

		stbi_set_flip_vertically_on_load(true);
		unsigned char* imageData = stbi_load_from_memory(data, size, &width, &height, &channels, 4);
		GA_CORE_ASSERT(imageData);

		CreateVulkanImage(imageData, width, height);
		stbi_image_free(imageData);
	}

	Texture::Texture(uint8_t* data, uint32_t width, uint32_t height)
	{
		CreateVulkanImage(data, width, height);
	}

	void Texture::CreateVulkanImage(unsigned char* data, int width, int height)
	{
		m_Image = Image::Create({ width, height }, ImageFormat::R8G8B8A8_UNORM, ImageUsage::Sampled | ImageUsage::TransferDst);
		Renderer::CopyImage(data, m_Image, width * height * 4);
	}

}