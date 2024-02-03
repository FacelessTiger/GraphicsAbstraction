#pragma once

#include <GraphicsAbstraction/Renderer/Image.h>
#include <Assets/Asset.h>

namespace GraphicsAbstraction {

	class Texture : public Asset
	{
	public:
		Texture(const char* filename);
		Texture(uint8_t* data, uint32_t size);
		Texture(uint8_t* data, uint32_t width, uint32_t height);
		virtual ~Texture() = default;

		inline const Ref<Image>& GetImage() const { return m_Image; }
		virtual AssetType GetType() const { return AssetType::Texture2D; }
	private:
		void CreateVulkanImage(unsigned char* data, int width, int height);
	private:
		Ref<Image> m_Image;
	};

}