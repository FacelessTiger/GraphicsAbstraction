#include <VulkanRHI.h>
#include <DxcCompiler.h>

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

		static ShaderStage VulkanStageToGA(VkShaderStageFlagBits stage)
		{
			switch (stage)
			{
				case VK_SHADER_STAGE_VERTEX_BIT:	return ShaderStage::Vertex;
				case VK_SHADER_STAGE_FRAGMENT_BIT:	return ShaderStage::Pixel;
				case VK_SHADER_STAGE_COMPUTE_BIT:	return ShaderStage::Compute;
			}

			assert(false && "Unknown shader type!");
			return (ShaderStage)0;
		}

	}

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
		if (g_ShaderObjectSupported) impl->Context->GetFrameDeletionQueue().Push(impl->ShaderObject);
		else impl->Context->GetFrameDeletionQueue().Push(impl->Module);

		s_ShaderList.erase(impl->ID);
		delete impl;
	}

	ShaderStage Shader::GetStage() const
	{
		return Utils::VulkanStageToGA(impl->Stage);
	}

	Impl<Shader>::Impl(const std::vector<uint32_t>& data, ShaderStage stage)
		: Context(Impl<GraphicsContext>::Reference), Stage(Utils::ShaderStageToVulkan(stage)), ID(s_IDCounter++)
	{
		CreatePipelineShaderStage(data);
		s_ShaderList[ID] = this;
	}

	Impl<Shader>::Impl(const std::string& path, ShaderStage stage, std::vector<uint32_t>* compiledData)
		: Context(Impl<GraphicsContext>::Reference), Path(path), Stage(Utils::ShaderStageToVulkan(stage)), ID(s_IDCounter++)
	{
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		std::string source(file.tellg(), 0);
		file.seekg(0);
		file.read(source.data(), source.size());

		auto spirv = DxcCompiler::CompileSource(path, source, stage, { L"-fspv-use-legacy-buffer-matrix-order", L"-spirv" }, { L"VK_BINDLESS" });
		if (compiledData) *compiledData = spirv;

		CreatePipelineShaderStage(spirv);
		s_ShaderList[ID] = this;
	}

	Impl<Shader>* Impl<Shader>::GetShaderByID(uint32_t id)
	{
		auto it = s_ShaderList.find(id);
		if (it == s_ShaderList.end()) return nullptr;

		return it->second;
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