#pragma once

#include <GraphicsAbstraction/Renderer/Shader.h>

#include <vulkan/vulkan.h>

#include <unordered_map>

namespace GraphicsAbstraction {

	class VulkanContext;

	class VulkanShader : public Shader
	{
	public:
		VulkanShader(std::shared_ptr<GraphicsContext> context, const std::string& filepath);
		virtual ~VulkanShader() = default;

		inline const std::vector<VkPipelineShaderStageCreateInfo>& GetPipelineShaderStages() const { return m_PipelineShaderStages; }
	private:
		std::string ReadFile(const std::string& filepath);
		std::unordered_map<VkShaderStageFlagBits, std::string> PreProcess(const std::string& source);

		void CompileOrGetVulkanBinaries(const std::unordered_map<VkShaderStageFlagBits, std::string>& shaderSources);
		void CreatePipelineShaderStages();
		void Reflect(VkShaderStageFlagBits stage, const std::vector<uint32_t>& shaderData);
	private:
		std::string m_FilePath;
		std::shared_ptr<VulkanContext> m_Context;

		std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>> m_VulkanSPIRV;
		std::vector<VkPipelineShaderStageCreateInfo> m_PipelineShaderStages;
	};

}