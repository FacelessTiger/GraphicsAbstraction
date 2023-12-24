#pragma once

#include <GraphicsAbstraction/Renderer/Image.h>

namespace GraphicsAbstraction {

	class Texture
	{
	public:
		Texture(const char* filename);

		inline const std::shared_ptr<Image>& GetImage() const { return m_Image; }
	private:
		std::shared_ptr<Image> m_Image;
	};

}