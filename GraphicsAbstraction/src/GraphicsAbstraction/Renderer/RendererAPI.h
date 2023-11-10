#pragma once

namespace GraphicsAbstraction {

	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0,
			Vulkan = 1
		};
	public:
		virtual ~RendererAPI() = default;

		inline static API GetAPI() { return s_API; }
	private:
		static API s_API;
	};

}