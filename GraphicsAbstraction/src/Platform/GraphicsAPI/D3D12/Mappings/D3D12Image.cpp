#include "D3D12Image.h"

namespace GraphicsAbstraction {

	Ref<Image> Image::Create(const glm::vec2& size, ImageFormat format, ImageUsage usage)
	{
		return CreateRef<D3D12Image>(size, format, usage);
	}

}