#pragma once

#include <GraphicsAbstraction/Renderer/Image.h>

namespace GraphicsAbstraction {

	class Texture
	{
	public:
		Texture(const char* filename);

		inline const Ref<Image>& GetImage() const { return m_Image; }
	private:
		Ref<Image> m_Image;
	};

}