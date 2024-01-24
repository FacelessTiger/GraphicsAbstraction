#include "ShaderManager.h"

#include <filesystem>
#include <fstream>

namespace GraphicsAbstraction {

	std::unordered_map<std::string, Ref<Shader>> ShaderManager::m_ShaderList;

	void ShaderManager::Init()
	{
#ifdef GA_DIST
		std::ifstream in("Assets/ShaderBlob" + GetExtension(), std::ios::in | std::ios::binary | std::ios::ate);
		uint32_t totalSize = in.tellg();
		in.seekg(0);

		uint32_t readSize = 0;
		while (readSize < totalSize)
		{
			ShaderStage stage;
			in.read((char*)&stage, sizeof(ShaderStage));
			readSize += sizeof(ShaderStage);

			uint32_t nameSize;
			in.read((char*)&nameSize, sizeof(uint32_t));

			std::string name(nameSize, '\0');
			in.read((char*)name.data(), nameSize);

			readSize += sizeof(uint32_t);
			readSize += nameSize;

			uint32_t blobSize;
			in.read((char*)&blobSize, sizeof(uint32_t));

			std::vector<uint32_t> blob(blobSize / sizeof(uint32_t));
			in.read((char*)blob.data(), blobSize);

			readSize += sizeof(uint32_t);
			readSize += blobSize;

			m_ShaderList[name] = Shader::Create(blob, stage);
		}
#endif
	}

	Ref<Shader> ShaderManager::Get(const std::string& name, ShaderStage stage)
	{
		if (m_ShaderList.contains(name)) return m_ShaderList[name];

		std::filesystem::path sourcePath = "Assets/shaders/" + name + ".hlsl";
		std::filesystem::path destPath = "Assets/cache/" + name + GetExtension();

		bool needsCompile = true;
		if (std::filesystem::exists(destPath))
			needsCompile = std::filesystem::last_write_time(destPath) < std::filesystem::last_write_time(sourcePath);

		Ref<Shader> shader;
		if (needsCompile)
		{
			std::vector<uint32_t> shaderData;
			shader = Shader::Create(sourcePath.string(), stage, &shaderData);

			std::ofstream out(destPath, std::ios::out | std::ios::binary);
			out.write((char*)shaderData.data(), shaderData.size() * sizeof(uint32_t));
			out.flush();
			out.close();
		}
		else
		{
			std::ifstream in(destPath, std::ios::in | std::ios::binary | std::ios::ate);
			auto size = in.tellg();
			in.seekg(0, std::ios::beg);

			std::vector<uint32_t> shaderData(size / sizeof(uint32_t));
			in.read((char*)shaderData.data(), size);

			shader = Shader::Create(shaderData, stage);
		}	

		m_ShaderList[name] = shader;
		return shader;
	}

	void ShaderManager::Serialize()
	{
#ifndef GA_DIST
		std::ofstream out("Assets/ShaderBlob" + GetExtension(), std::ios::out | std::ios::binary);
		for (auto& [name, shader] : m_ShaderList)
		{
			ShaderStage stage = shader->GetStage();
			out.write((char*)&stage, sizeof(ShaderStage));

			uint32_t nameSize = name.size();
			out.write((char*)&nameSize, sizeof(uint32_t));
			out.write(name.data(), nameSize);

			std::ifstream in("Assets/cache/" + name + GetExtension(), std::ios::in | std::ios::binary | std::ios::ate);
			auto size = in.tellg();
			in.seekg(0, std::ios::beg);

			std::vector<uint32_t> shaderData(size / sizeof(uint32_t));
			in.read((char*)shaderData.data(), size);

			out.write((char*)&size, sizeof(uint32_t));
			out.write((char*)shaderData.data(), size);
		}
#endif
	}

	std::string ShaderManager::GetExtension()
	{
		if (GraphicsContext::GetShaderCompiledType() == ShaderCompiledType::Spirv) return ".spirv";
		else return ".dxil";
	}

}