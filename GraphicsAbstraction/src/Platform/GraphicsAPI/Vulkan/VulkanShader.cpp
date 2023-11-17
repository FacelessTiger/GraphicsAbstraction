#include "VulkanShader.h"

#include <Platform/GraphicsAPI/Vulkan/VulkanContext.h>
#include <GraphicsAbstraction/Debug/Instrumentor.h>

#include <fstream>
#include <filesystem>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

namespace GraphicsAbstraction {

	namespace Utils {

		static VkShaderStageFlagBits ShaderTypeFromString(const std::string& type)
		{
			if (type == "vertex")						return VK_SHADER_STAGE_VERTEX_BIT;
			if (type == "fragment" || type == "pixel")	return VK_SHADER_STAGE_FRAGMENT_BIT;

			GA_CORE_ASSERT(false, "Unknown shader type!");
			return (VkShaderStageFlagBits)0;
		}

		static shaderc_shader_kind ShaderStageToShaderC(VkShaderStageFlagBits stage)
		{
			switch (stage)
			{
				case VK_SHADER_STAGE_VERTEX_BIT:	return shaderc_glsl_vertex_shader;
				case VK_SHADER_STAGE_FRAGMENT_BIT:	return shaderc_glsl_fragment_shader;
			}

			GA_CORE_ASSERT(false);
			return (shaderc_shader_kind)0;
		}

		static const char* ShaderStageToString(VkShaderStageFlagBits stage)
		{
			switch (stage)
			{
				case VK_SHADER_STAGE_VERTEX_BIT:	return "VK_SHADER_STAGE_VERTEX_BIT";
				case VK_SHADER_STAGE_FRAGMENT_BIT:	return "VK_SHADER_STAGE_FRAGMENT_BIT";
			}

			GA_CORE_ASSERT(false);
			return nullptr;
		}

		static const char* GetCacheDirectory()
		{
			return "Assets/cache/shader/vulkan";
		}

		static void CreateCacheDirectoryIfNeeded()
		{
			std::string cacheDirectory = GetCacheDirectory();
			if (!std::filesystem::exists(cacheDirectory))
				std::filesystem::create_directories(cacheDirectory);
		}

		static const char* ShaderStageCachedVulkanFileExtension(VkShaderStageFlagBits stage)
		{
			switch (stage)
			{
				case VK_SHADER_STAGE_VERTEX_BIT: return ".cached_vulkan.vert";
				case VK_SHADER_STAGE_FRAGMENT_BIT: return ".cached_vulkan.frag";
			}

			GA_CORE_ASSERT(false);
			return "";
		}

	}

	VulkanShader::VulkanShader(std::shared_ptr<GraphicsContext> context, const std::string& filepath)
		: m_FilePath(filepath), m_Context(std::dynamic_pointer_cast<VulkanContext>(context))
	{
		GA_PROFILE_SCOPE();
		Utils::CreateCacheDirectoryIfNeeded();

		std::string source = ReadFile(filepath);
		auto shaderSources = PreProcess(source);

		CompileOrGetVulkanBinaries(shaderSources);
		CreatePipelineShaderStages();
	}

	std::string VulkanShader::ReadFile(const std::string& filepath)
	{
		GA_PROFILE_SCOPE();

		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary);

		if (in)
		{
			in.seekg(0, std::ios::end);

			size_t size = in.tellg();
			if (size != -1)
			{
				result.resize(in.tellg());

				in.seekg(0, std::ios::beg);
				in.read(&result[0], result.size());

				in.close();
			}
			else
			{
				GA_CORE_ERROR("Could not open file \"{0}\"", filepath);
			}
		}
		else
		{
			GA_CORE_ERROR("Could not open file \"{0}\"", filepath);
		}

		return result;
	}

	std::unordered_map<VkShaderStageFlagBits, std::string> VulkanShader::PreProcess(const std::string& source)
	{
		GA_PROFILE_SCOPE();
		std::unordered_map<VkShaderStageFlagBits, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);

		size_t pos = source.find(typeToken, 0);
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos);
			GA_CORE_ASSERT(eol != std::string::npos, "Syntax error");

			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);

			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			GA_CORE_ASSERT(nextLinePos != std::string::npos, "Syntax error");

			pos = source.find(typeToken, nextLinePos);
			shaderSources[Utils::ShaderTypeFromString(type)] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
		}

		return shaderSources;
	}

	void VulkanShader::CompileOrGetVulkanBinaries(const std::unordered_map<VkShaderStageFlagBits, std::string>& shaderSources)
	{
		GA_PROFILE_SCOPE();

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
		const bool optimize = true;
		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

		auto& shaderData = m_VulkanSPIRV;
		shaderData.clear();

		for (auto&& [stage, source] : shaderSources)
		{
			std::filesystem::path shaderFilePath = m_FilePath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::ShaderStageCachedVulkanFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);

			bool needsCompile = true;
			if (in.is_open())
				needsCompile = std::filesystem::last_write_time(m_FilePath) > std::filesystem::last_write_time(cachedPath);		

			if (needsCompile)
			{
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::ShaderStageToShaderC(stage), m_FilePath.c_str(), options);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					GA_CORE_ERROR(module.GetErrorMessage());
					GA_CORE_ASSERT(false);
				}

				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = shaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
			else
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
		}

		for (auto&& [stage, data] : shaderData)
			Reflect(stage, data);
	}

	void VulkanShader::CreatePipelineShaderStages()
	{
		GA_PROFILE_SCOPE();

		for (auto&& [stage, data] : m_VulkanSPIRV)
		{
			VkShaderModuleCreateInfo moduleCreateInfo = {};
			moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			moduleCreateInfo.pNext = nullptr;
			moduleCreateInfo.codeSize = data.size() * sizeof(uint32_t);
			moduleCreateInfo.pCode = data.data();

			VkShaderModule shaderModule;
			VK_CHECK(vkCreateShaderModule(m_Context->GetLogicalDevice(), &moduleCreateInfo, nullptr, &shaderModule));

			VkPipelineShaderStageCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			info.pNext = nullptr;
			info.stage = stage;
			info.module = shaderModule;
			info.pName = "main"; // hard coding entry point to main
			m_PipelineShaderStages.push_back(info);

			m_Context->PushToDeletionQueue([module = shaderModule](VulkanContext& context) {
				vkDestroyShaderModule(context.GetLogicalDevice(), module, nullptr);
			});
		}
	}

	void VulkanShader::Reflect(VkShaderStageFlagBits stage, const std::vector<uint32_t>& shaderData)
	{
		spirv_cross::Compiler compiler(shaderData);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		GA_CORE_TRACE("VulkanShader::Reflect - {0} {1}", Utils::ShaderStageToString(stage), m_FilePath);
		GA_CORE_TRACE("    {0} uniform buffers", resources.uniform_buffers.size());
		GA_CORE_TRACE("    {0} resources", resources.sampled_images.size());

		GA_CORE_TRACE("Uniform buffers:");
		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			uint32_t bufferSize = compiler.get_declared_struct_size(bufferType);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			int memberCount = bufferType.member_types.size();

			GA_CORE_TRACE("  {0}", resource.name);
			GA_CORE_TRACE("    Size = {0}", bufferSize);
			GA_CORE_TRACE("    Binding = {0}", binding);
			GA_CORE_TRACE("    Members = {0}", memberCount);
		}
	}

}