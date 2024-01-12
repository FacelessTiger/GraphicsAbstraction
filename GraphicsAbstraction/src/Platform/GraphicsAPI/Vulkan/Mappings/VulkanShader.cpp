#include "VulkanShader.h"

#include <GraphicsAbstraction/Debug/Instrumentor.h>
#include <GraphicsAbstraction/Core/Log.h>
#include <GraphicsAbstraction/Core/Assert.h>

#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanImage.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanContext.h>

#include <filesystem>
#include <fstream>

#include <wtypes.h>
#include <Unknwn.h>
#include <wrl/client.h>
#include <dxc/dxcapi.h>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_hlsl.hpp>

namespace GraphicsAbstraction {

	static uint32_t s_IDCounter = 1;
	static std::unordered_map<uint32_t, VulkanShader*> s_ShaderList;

	namespace Utils {

		static VkShaderStageFlagBits ShaderStageToVulkan(ShaderStage stage)
		{
			switch (stage)
			{
				case ShaderStage::Vertex:	return VK_SHADER_STAGE_VERTEX_BIT;
				case ShaderStage::Pixel:	return VK_SHADER_STAGE_FRAGMENT_BIT;
				case ShaderStage::Compute:	return VK_SHADER_STAGE_COMPUTE_BIT;
			}

			GA_CORE_ASSERT(false, "Unknown shader type!");
			return (VkShaderStageFlagBits)0;
		}

		static LPCWSTR ShaderStageToDXC(VkShaderStageFlagBits stage)
		{
			switch (stage)
			{
				case VK_SHADER_STAGE_VERTEX_BIT:	return L"vs_6_5";
				case VK_SHADER_STAGE_FRAGMENT_BIT:	return L"ps_6_5";
				case VK_SHADER_STAGE_COMPUTE_BIT:	return L"cs_6_5";
			}

			GA_CORE_ASSERT(false);
			return nullptr;
		}

		static const char* ShaderStageToString(VkShaderStageFlagBits stage)
		{
			switch (stage)
			{
				case VK_SHADER_STAGE_VERTEX_BIT:	return "VK_SHADER_STAGE_VERTEX_BIT";
				case VK_SHADER_STAGE_FRAGMENT_BIT:	return "VK_SHADER_STAGE_FRAGMENT_BIT";
				case VK_SHADER_STAGE_COMPUTE_BIT:	return "VK_SHADER_STAGE_COMPUTE_BIT";
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
				case VK_SHADER_STAGE_VERTEX_BIT:	return ".cached_vulkan.vert";
				case VK_SHADER_STAGE_FRAGMENT_BIT:	return ".cached_vulkan.frag";
				case VK_SHADER_STAGE_COMPUTE_BIT:	return ".cached_vulkan.compute";
			}

			GA_CORE_ASSERT(false);
			return "";
		}

	}

	Ref<Shader> Shader::Create(const std::string& path, ShaderStage stage)
	{
		return CreateRef<VulkanShader>(path, stage);
	}

	VulkanShader::VulkanShader(const std::string& path, ShaderStage stage)
		: m_Context(VulkanContext::GetReference()), m_Path(path), Stage(Utils::ShaderStageToVulkan(stage)), ID(s_IDCounter++)
	{
		GA_PROFILE_SCOPE();
		Utils::CreateCacheDirectoryIfNeeded();

		auto spirv = CompileOrGetVulkanBinaries();
		CreatePipelineShaderStage(spirv);

		s_ShaderList[ID] = this;
	}

	VulkanShader::~VulkanShader()
	{
		if (m_Context->ShaderObjectSupported) m_Context->GetFrameDeletionQueue().Push(ShaderObject);
		else m_Context->GetFrameDeletionQueue().Push(Module);

		s_ShaderList.erase(ID);
	}

	VulkanShader* VulkanShader::GetShaderByID(uint32_t id)
	{
		auto it = s_ShaderList.find(id);
		if (it == s_ShaderList.end()) return nullptr;

		return it->second;
	}

	std::vector<uint32_t> VulkanShader::CompileOrGetVulkanBinaries()
	{
		GA_PROFILE_SCOPE();

		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();
		std::vector<uint32_t> shaderData;

		std::filesystem::path shaderFilePath = m_Path;
		std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::ShaderStageCachedVulkanFileExtension(Stage));

		std::ifstream in(cachedPath, std::ios::in | std::ios::binary);

		bool needsCompile = true;
		if (in.is_open())
			needsCompile = std::filesystem::last_write_time(m_Path) > std::filesystem::last_write_time(cachedPath);

		if (needsCompile)
		{
			shaderData = CompileVulkanBinaries();

			std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
			if (out.is_open())
			{
				auto& data = shaderData;
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

			auto& data = shaderData;
			data.resize(size / sizeof(uint32_t));
			in.read((char*)data.data(), size);
		}

		Reflect(shaderData);
		return shaderData;
	}

	std::vector<uint32_t> VulkanShader::CompileVulkanBinaries()
	{
		HRESULT hres;

		IDxcUtils* utils;
		hres = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
		GA_CORE_ASSERT(SUCCEEDED(hres));

		IDxcCompiler3* compiler;
		hres = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
		GA_CORE_ASSERT(SUCCEEDED(hres));

		std::wstring wstring = std::filesystem::path(m_Path).wstring();
		LPCWSTR filePath = wstring.c_str();

		uint32_t codePage = DXC_CP_ACP;
		IDxcBlobEncoding* sourceBlob;
		hres = utils->LoadFile(filePath, &codePage, &sourceBlob);

		if (FAILED(hres))
		{
			GA_CORE_ERROR("Could not open file \"{0}\"", m_Path);
			GA_CORE_ASSERT(false);
		}

		IDxcIncludeHandler* handler;
		utils->CreateDefaultIncludeHandler(&handler);
		
		// Note: Zi causes debug information
		std::vector<LPCWSTR> arguments = {
			filePath, L"-fspv-use-legacy-buffer-matrix-order",
			L"-E", L"main", 
			L"-D", L"VK_BINDLESS",
			L"-T", Utils::ShaderStageToDXC(Stage),
			L"-spirv",
			L"-Zi"
		};

		DxcBuffer buffer{};
		buffer.Encoding = DXC_CP_ACP;
		buffer.Ptr = sourceBlob->GetBufferPointer();
		buffer.Size = sourceBlob->GetBufferSize();
		
		IDxcResult* result;
		hres = compiler->Compile(&buffer, arguments.data(), (uint32_t)arguments.size(), handler, IID_PPV_ARGS(&result));
		result->GetStatus(&hres);

		if (FAILED(hres) && result)
		{
			IDxcBlobEncoding* errorBlob;
			result->GetErrorBuffer(&errorBlob);

			GA_CORE_ERROR((const char*)errorBlob->GetBufferPointer());
			GA_CORE_ASSERT(false);
		}

		IDxcBlob* code;
		result->GetResult(&code);

		std::vector<uint32_t> ret(code->GetBufferSize() / sizeof(uint32_t));
		memcpy(ret.data(), code->GetBufferPointer(), code->GetBufferSize());
		return ret;
	}

	void VulkanShader::CreatePipelineShaderStage(const std::vector<uint32_t>& data)
	{
		if (m_Context->ShaderObjectSupported)
		{
			VkShaderCreateInfoEXT shaderInfo = {
				.sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
				.stage = Stage,
				.nextStage = (Stage == VK_SHADER_STAGE_VERTEX_BIT) ? VK_SHADER_STAGE_FRAGMENT_BIT : (VkShaderStageFlags)0,
				.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT,
				.codeSize = data.size() * sizeof(uint32_t),
				.pCode = data.data(),
				.pName = "main", // hard coding entry point to main
				.setLayoutCount = 1,
				.pSetLayouts = &m_Context->BindlessSetLayout,
				.pushConstantRangeCount = (uint32_t)m_Context->PushConstantRanges.size(),
				.pPushConstantRanges = m_Context->PushConstantRanges.data()
			};
			m_Context->vkCreateShadersEXT(m_Context->Device, 1, &shaderInfo, nullptr, &ShaderObject);
		}
		else
		{
			VkShaderModuleCreateInfo moduleInfo = {
				.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.codeSize = data.size() * sizeof(uint32_t),
				.pCode = data.data()
			};
			vkCreateShaderModule(m_Context->Device, &moduleInfo, nullptr, &Module);

			StageInfo = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage = Stage,
				.module = Module,
				.pName = "main" // hard coding entry point to main
			};
		}
	}

	void VulkanShader::Reflect(const std::vector<uint32_t>& data)
	{
		spirv_cross::Compiler compiler(data);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();
		
		GA_CORE_TRACE("VulkanShader::Reflect - {0} {1}", Utils::ShaderStageToString(Stage), m_Path);
		GA_CORE_TRACE("    {0} uniform buffers", resources.uniform_buffers.size());
		GA_CORE_TRACE("    {0} push constants", resources.push_constant_buffers.size());
		GA_CORE_TRACE("    {0} resources", resources.sampled_images.size());

		GA_CORE_TRACE("Uniform buffers:");
		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			size_t bufferSize = compiler.get_declared_struct_size(bufferType);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			size_t memberCount = bufferType.member_types.size();

			GA_CORE_TRACE("  {0}", resource.name);
			GA_CORE_TRACE("    Size = {0}", bufferSize);
			GA_CORE_TRACE("    Binding = {0}", binding);
			GA_CORE_TRACE("    Members = {0}", memberCount);
		}

		GA_CORE_TRACE("Push constants:");
		for (const auto& resource : resources.push_constant_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			size_t bufferSize = compiler.get_declared_struct_size(bufferType);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			size_t memberCount = bufferType.member_types.size();

			GA_CORE_TRACE("  {0}", resource.name);
			GA_CORE_TRACE("    Size = {0}", bufferSize);
			GA_CORE_TRACE("    Binding = {0}", binding);
			GA_CORE_TRACE("    Members = {0}", memberCount);
		}
	}

}