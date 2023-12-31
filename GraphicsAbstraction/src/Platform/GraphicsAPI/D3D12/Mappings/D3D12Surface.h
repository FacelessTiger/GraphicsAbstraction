#pragma once

#include <GraphicsAbstraction/Renderer/Surface.h>
#include <GraphicsAbstraction/Core/Window.h>

namespace GraphicsAbstraction {

	class D3D12Surface : public Surface
	{
	public:
		D3D12Surface(const Ref<Window>& window) { }
	};

}