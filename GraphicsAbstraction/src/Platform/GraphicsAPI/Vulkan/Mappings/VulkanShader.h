#pragma once

#include <GraphicsAbstraction/Renderer/Shader.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanContext.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanBuffer.h>

#include <vulkan/vulkan.h>

namespace GraphicsAbstraction {

	class VulkanContext;

	class VulkanShader : public Shader
	{
	public:
		VkShaderStageFlagBits Stage;
		uint32_t ID;

		VkShaderEXT ShaderObject;
		VkShaderModule Module;
		VkPipelineShaderStageCreateInfo StageInfo;
	public:
		VulkanShader(const std::string& path, ShaderStage stage);
		virtual ~VulkanShader();

		static VulkanShader* GetShaderByID(uint32_t id);
	private:
		std::vector<uint32_t> CompileOrGetVulkanBinaries();
		std::vector<uint32_t> CompileVulkanBinaries();
		void CreatePipelineShaderStage(const std::vector<uint32_t>& data);

		void Reflect(const std::vector<uint32_t>& data);
	private:
		Ref<VulkanContext> m_Context;
		std::string m_Path;
	};

}