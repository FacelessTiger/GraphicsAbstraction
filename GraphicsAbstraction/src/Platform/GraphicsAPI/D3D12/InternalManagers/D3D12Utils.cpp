#include "D3D12Utils.h"

#include <GraphicsAbstraction/Core/Log.h>

namespace GraphicsAbstraction::Utils {

	DXGI_FORMAT GAImageFormatToD3D12(ImageFormat format)
	{
		switch (format)
		{
			case ImageFormat::Unknown:				return DXGI_FORMAT_UNKNOWN;
			case ImageFormat::R16G16B16A16_SFLOAT:	return DXGI_FORMAT_R16G16B16A16_FLOAT;
			case ImageFormat::R8G8B8A8_UNORM:		return DXGI_FORMAT_R8G8B8A8_UNORM;
			case ImageFormat::B8G8R8A8_SRGB:		return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
			case ImageFormat::D32_SFLOAT:			return DXGI_FORMAT_D32_FLOAT;
		}

		GA_CORE_ASSERT(false, "Unknown image format!");
		return DXGI_FORMAT_UNKNOWN;
	}

}