#pragma once

#include <GraphicsAbstraction/Renderer/Shader.h>
#include <Platform/GraphicsAPI/Vulkan/VulkanContext.h>
#include <Platform/GraphicsAPI/Vulkan/VulkanBuffer.h>

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

		void WriteImage(const std::shared_ptr<Image> image, uint32_t index) override;
		void WriteBuffer(const std::shared_ptr<Buffer> buffer, uint32_t index) override;

		uint32_t GetPushConstantBufferID();
		static VulkanShader* GetShaderByID(uint32_t id);
	private:
		std::vector<uint32_t> CompileOrGetVulkanBinaries();
		std::vector<uint32_t> CompileVulkanBinaries();
		void CreatePipelineShaderStage(const std::vector<uint32_t>& data);
		void CreateDescriptorBuffer();

		void Reflect(const std::vector<uint32_t>& data);
	private:
		VulkanContextReference m_Context;
		std::string m_Path;

		std::vector<uint32_t> m_PushConstants;
		bool m_PushConstantsDirty = false;

		std::shared_ptr<VulkanBuffer> m_PushConstantBuffer;
	};

}