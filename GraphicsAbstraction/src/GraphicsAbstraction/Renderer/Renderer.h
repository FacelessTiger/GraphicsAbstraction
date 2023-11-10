#pragma once

#include "RendererAPI.h"

namespace GraphicsAbstraction {

	class Renderer
	{
	public:
		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	};

}