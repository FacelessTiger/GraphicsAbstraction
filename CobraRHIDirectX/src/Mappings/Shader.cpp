#include <DirectXRHI.h>
#include <DxcCompiler.h>

#include <filesystem>
#include <fstream>

namespace GraphicsAbstraction {

	namespace Utils {

		static LPCWSTR ShaderStageToDXC(ShaderStage stage)
		{
			switch (stage)
			{
				case ShaderStage::Vertex:	return L"vs_6_6";
				case ShaderStage::Pixel:	return L"ps_6_6";
				case ShaderStage::Compute:	return L"cs_6_6";
			}

			assert(false);
			return nullptr;
		}

	}

	static uint32_t s_IDCounter = 1;
	static std::unordered_map<uint32_t, Impl<Shader>*> s_ShaderList;

	Ref<Shader> Shader::Create(const std::vector<uint32_t>& data, ShaderStage stage)
	{
		auto shader = CreateRef<Shader>();
		shader->impl = new Impl<Shader>(data, stage);
		return shader;
	}

	Ref<Shader> Shader::Create(const std::string& path, ShaderStage stage, std::vector<uint32_t>* compiledData)
	{
		auto shader = CreateRef<Shader>();
		shader->impl = new Impl<Shader>(path, stage, compiledData);
		return shader;
	}

	Shader::~Shader()
	{
		s_ShaderList.erase(impl->ID);
		delete impl;
	}

	Impl<Shader>::Impl(const std::vector<uint32_t>& data, ShaderStage stage)
		: Stage(stage), ID(s_IDCounter++)
	{
		Source = data;
		Shader = {
			.pShaderBytecode = Source.data(),
			.BytecodeLength = Source.size() * sizeof(uint32_t)
		};
		s_ShaderList[ID] = this;
	}

	Impl<Shader>::Impl(const std::string& path, ShaderStage stage, std::vector<uint32_t>* compiledData)
		: Stage(stage), ID(s_IDCounter++)
	{
		std::string source = ReadAndPreProcessFile(path);
		Source = DxcCompiler::CompileSource(path, source, stage, {}, {});
		if (compiledData) *compiledData = Source;

		Shader = {
			.pShaderBytecode = Source.data(),
			.BytecodeLength = Source.size() * sizeof(uint32_t)
		};
		s_ShaderList[ID] = this;
	}

	Impl<Shader>* Impl<Shader>::GetShaderByID(uint32_t id)
	{
		auto it = s_ShaderList.find(id);
		if (it == s_ShaderList.end()) return nullptr;

		return it->second;
	}

	std::string Impl<Shader>::ReadAndPreProcessFile(const std::string& path)
	{
		std::ifstream t(path);
		std::stringstream buffer;
		buffer << t.rdbuf();
		std::string file = buffer.str();

		// TODO: logic below only works if sv_vertexid is declared in arugments, if it's an input struct it won't work
		if (Stage == ShaderStage::Vertex)
		{
			int semantic = file.find("SV_VertexID");
			if (semantic != std::string::npos)
			{
				int colonAfter = file.find_last_of(':', semantic);
				int spaceBefore = file.find_last_of(' ', colonAfter) + 1;
				std::string variableName = file.substr(spaceBefore, colonAfter - spaceBefore);

				file.replace(spaceBefore, colonAfter - spaceBefore, "g_LocalVertexID");
				int colonIndex = file.find("{", colonAfter);

				file.insert(colonIndex + 1, "uint " + variableName + " = g_LocalVertexID + g_SpecialConstants.vertexOffset;");
			}

			semantic = file.find("SV_InstanceID");
			if (semantic != std::string::npos)
			{
				int colonAfter = file.find_last_of(':', semantic);
				int spaceBefore = file.find_last_of(' ', colonAfter) + 1;
				std::string variableName = file.substr(spaceBefore, colonAfter - spaceBefore);

				file.replace(spaceBefore, colonAfter - spaceBefore, "g_LocalInstanceID");
				int colonIndex = file.find("{", colonAfter);

				file.insert(colonIndex + 1, "uint " + variableName + " = g_LocalInstanceID + g_SpecialConstants.instanceOffset;");
			}
		}

		return file;
	}

}