#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include <GraphicsAbstraction/Renderer/Image.h>

namespace GraphicsAbstraction::Utils {

	DXGI_FORMAT GAImageFormatToD3D12(ImageFormat format);

}