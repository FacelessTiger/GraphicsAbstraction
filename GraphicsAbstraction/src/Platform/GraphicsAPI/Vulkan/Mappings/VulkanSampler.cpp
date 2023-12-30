#include "VulkanSampler.h"

#include <GraphicsAbstraction/Core/Assert.h>

namespace GraphicsAbstraction {

	namespace Utils {

		VkFilter GAFilterToVulkan(Filter filter)
		{
			switch (filter)
			{
				case Filter::Nearest:	return VK_FILTER_NEAREST;
				case Filter::Linear:	return VK_FILTER_LINEAR;
			}

			GA_CORE_ASSERT(false, "Unknown filter!");
			return (VkFilter)0;
		}

	}

	Ref<Sampler> Sampler::Create(Filter min, Filter mag)
	{
		return CreateRef<VulkanSampler>(min, mag);
	}

	VulkanSampler::VulkanSampler(Filter min, Filter mag)
		: m_Context(VulkanContext::GetReference())
	{
		VkSamplerCreateInfo info = {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter = Utils::GAFilterToVulkan(mag),
			.minFilter = Utils::GAFilterToVulkan(min)
		};

		vkCreateSampler(m_Context->Device, &info, nullptr, &Sampler);

		// update descriptor
		VkDescriptorImageInfo imageInfo = {
			.sampler = Sampler
		};

		VkWriteDescriptorSet write = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = m_Context->BindlessSet,
			.dstBinding = m_Context->SAMPLER_BINDING,
			.dstArrayElement = Handle.GetValue(),
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
			.pImageInfo = &imageInfo,
		};

		vkUpdateDescriptorSets(m_Context->Device, 1, &write, 0, nullptr);
	}

	VulkanSampler::~VulkanSampler()
	{
		m_Context->GetFrameDeletionQueue().Push(Sampler);
	}

}