#include "D3D12Utils.h"

#include <GraphicsAbstraction/Core/Log.h>

namespace GraphicsAbstraction::Utils {

	DXGI_FORMAT GAImageFormatToD3D12(ImageFormat format)
	{
		switch (format)
		{
			case ImageFormat::R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
		}

		GA_CORE_ASSERT(false, "Unknown image format!");
		return DXGI_FORMAT_UNKNOWN;
	}

}