#include <VulkanRHI.h>

#include <filesystem>
#include <fstream>

#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_hlsl.hpp>
#include <wtypes.h>
#include <Unknwn.h>
#include <wrl/client.h>
#include <dxc/dxcapi.h>

namespace GraphicsAbstraction {

	static uint32_t s_IDCounter = 1;
	static std::unordered_map<uint32_t, Impl<Shader>*> s_ShaderList;

	namespace Utils {

		static VkShaderStageFlagBits ShaderStageToVulkan(ShaderStage stage)
		{
			switch (stage)
			{
				case ShaderStage::Vertex:	return VK_SHADER_STAGE_VERTEX_BIT;
				case ShaderStage::Pixel:	return VK_SHADER_STAGE_FRAGMENT_BIT;
				case ShaderStage::Compute:	return VK_SHADER_STAGE_COMPUTE_BIT;
			}

			assert(false && "Unknown shader type!");
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

			assert(false);
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

			assert(false);
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

			assert(false);
			return "";
		}

	}

	Ref<Shader> Shader::Create(const std::string& path, ShaderStage stage)
	{
		auto shader = CreateRef<Shader>();
		shader->impl = new Impl<Shader>(path, stage);
		return shader;
	}

	Shader::~Shader()
	{
		if (g_ShaderObjectSupported) impl->Context->GetFrameDeletionQueue().Push(impl->ShaderObject);
		else impl->Context->GetFrameDeletionQueue().Push(impl->Module);

		s_ShaderList.erase(impl->ID);
		delete impl;
	}

	Impl<Shader>::Impl(const std::string& path, ShaderStage stage)
		: Context(Impl<GraphicsContext>::Reference), Path(path), Stage(Utils::ShaderStageToVulkan(stage)), ID(s_IDCounter++)
	{
		Utils::CreateCacheDirectoryIfNeeded();

		auto spirv = CompileOrGetVulkanBinaries();
		CreatePipelineShaderStage(spirv);

		s_ShaderList[ID] = this;
	}

	Impl<Shader>* Impl<Shader>::GetShaderByID(uint32_t id)
	{
		auto it = s_ShaderList.find(id);
		if (it == s_ShaderList.end()) return nullptr;

		return it->second;
	}

	std::vector<uint32_t> Impl<Shader>::CompileOrGetVulkanBinaries()
	{
		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();
		std::vector<uint32_t> shaderData;

		std::filesystem::path shaderFilePath = Path;
		std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::ShaderStageCachedVulkanFileExtension(Stage));

		std::ifstream in(cachedPath, std::ios::in | std::ios::binary);

		bool needsCompile = true;
		if (in.is_open())
			needsCompile = std::filesystem::last_write_time(Path) > std::filesystem::last_write_time(cachedPath);

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

		return shaderData;
	}

	std::vector<uint32_t> Impl<Shader>::CompileVulkanBinaries()
	{
		HRESULT hres;

		IDxcUtils* utils;
		hres = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
		assert(SUCCEEDED(hres));

		IDxcCompiler3* compiler;
		hres = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
		assert(SUCCEEDED(hres));

		std::wstring wstring = std::filesystem::path(Path).wstring();
		LPCWSTR filePath = wstring.c_str();

		uint32_t codePage = DXC_CP_ACP;
		IDxcBlobEncoding* sourceBlob;
		hres = utils->LoadFile(filePath, &codePage, &sourceBlob);
		assert(SUCCEEDED(hres) && ("Could not open file " + Path).c_str());

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

			assert(false && (const char*)errorBlob->GetBufferPointer());
		}

		IDxcBlob* code;
		result->GetResult(&code);

		std::vector<uint32_t> ret(code->GetBufferSize() / sizeof(uint32_t));
		memcpy(ret.data(), code->GetBufferPointer(), code->GetBufferSize());
		return ret;
	}

	void Impl<Shader>::CreatePipelineShaderStage(const std::vector<uint32_t>& data)
	{
		if (g_ShaderObjectSupported)
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
				.pSetLayouts = &Context->BindlessSetLayout,
				.pushConstantRangeCount = (uint32_t)PUSH_CONSTANT_RANGES.size(),
				.pPushConstantRanges = PUSH_CONSTANT_RANGES.data()
			};
			Context->vkCreateShadersEXT(Context->Device, 1, &shaderInfo, nullptr, &ShaderObject);
		}
		else
		{
			VkShaderModuleCreateInfo moduleInfo = {
				.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.codeSize = data.size() * sizeof(uint32_t),
				.pCode = data.data()
			};
			vkCreateShaderModule(Context->Device, &moduleInfo, nullptr, &Module);

			StageInfo = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage = Stage,
				.module = Module,
				.pName = "main" // hard coding entry point to main
			};
		}
	}

}