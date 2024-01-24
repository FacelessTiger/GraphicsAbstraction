#pragma once

#include <string>
#include <unordered_map>

#include <GraphicsAbstraction/GraphicsAbstraction.h>

namespace GraphicsAbstraction {

	class ShaderManager
	{
	public:
		static void Init();

		static Ref<Shader> Get(const std::string& name, ShaderStage stage);
		static void Serialize();
	private:
		static std::string GetExtension();
	private:
		static std::unordered_map<std::string, Ref<Shader>> m_ShaderList;
	};

}