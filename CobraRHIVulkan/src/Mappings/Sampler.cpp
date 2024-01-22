#include <VulkanRHI.h>

namespace GraphicsAbstraction {

	namespace Utils {

		VkFilter GAFilterToVulkan(Filter filter)
		{
			switch (filter)
			{
				case Filter::Nearest:	return VK_FILTER_NEAREST;
				case Filter::Linear:	return VK_FILTER_LINEAR;
			}

			assert(false && "Unknown filter!");
			return (VkFilter)0;
		}

	}

	Ref<Sampler> Sampler::Create(Filter min, Filter mag)
	{
		auto sampler = CreateRef<Sampler>();
		sampler->impl = new Impl<Sampler>(min, mag);
		return sampler;
	}

	Sampler::~Sampler()
	{
		impl->Context->GetFrameDeletionQueue().Push(impl->Sampler);
		delete impl;
	}

	uint32_t Sampler::GetHandle() const
	{
		return impl->Handle.GetValue();
	}

	Impl<Sampler>::Impl(Filter min, Filter mag)
		: Context(Impl<GraphicsContext>::Reference)
	{
		VkSamplerCreateInfo info = {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter = Utils::GAFilterToVulkan(mag),
			.minFilter = Utils::GAFilterToVulkan(min)
		};

		vkCreateSampler(Context->Device, &info, nullptr, &Sampler);

		// update descriptor
		VkDescriptorImageInfo imageInfo = {
			.sampler = Sampler
		};

		VkWriteDescriptorSet write = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = Context->BindlessSet,
			.dstBinding = SAMPLER_BINDING,
			.dstArrayElement = Handle.GetValue(),
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
			.pImageInfo = &imageInfo,
		};

		vkUpdateDescriptorSets(Context->Device, 1, &write, 0, nullptr);
	}

}