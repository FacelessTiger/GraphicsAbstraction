#include "VulkanPushConstant.h"

#include <vulkan/vulkan.h>

#include <GraphicsAbstraction/Debug/Instrumentor.h>
#include <Platform/GraphicsAPI/Vulkan/VulkanCommandBuffer.h>
#include <Platform/GraphicsAPI/Vulkan/VulkanPipeline.h>

namespace GraphicsAbstraction {

	namespace Utils {

		static VkShaderStageFlags GAShaderStageToVulkan(PushConstant::ShaderStage stage)
		{
			switch (stage)
			{
			case PushConstant::ShaderStage::Vertex: return VK_SHADER_STAGE_VERTEX_BIT;
			}

			GA_CORE_ASSERT(false, "Unknown shader stage!");
			return VK_SHADER_STAGE_ALL;
		}

	}

	void VulkanPushConstant::Push(std::shared_ptr<CommandBuffer> cmd, std::shared_ptr<Pipeline> pipeline, void* data)
	{
		std::shared_ptr<VulkanCommandBuffer> vulkanCommandBuffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(cmd);
		std::shared_ptr<VulkanPipeline> vulkanPipeline = std::dynamic_pointer_cast<VulkanPipeline>(pipeline);
		
		vkCmdPushConstants(vulkanCommandBuffer->GetInternal(), vulkanPipeline->GetLayout(), Utils::GAShaderStageToVulkan(m_Stage), m_Offset, m_Size, data);
	}

}