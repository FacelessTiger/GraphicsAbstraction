#pragma once

#include <GraphicsAbstraction/Renderer/Buffer.h>
#include <Platform/GraphicsAPI/Vulkan/VulkanContext.h>
#include <Platform/GraphicsAPI/Vulkan/VulkanResourceHandle.h>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace GraphicsAbstraction {

	class VulkanContext;

	class VulkanBuffer : public Buffer
	{
	public:
		uint32_t Size;
		Utils::AllocatedBuffer Buffer;

		VulkanResourceHandle Handle = { ResourceType::StorageBuffer };
	public:
		VulkanBuffer(uint32_t size, BufferUsage usage, BufferFlags flags);
		virtual ~VulkanBuffer();

		void SetData(const void* data, uint32_t size = 0, uint32_t offset = 0) override;
		void SetData(const std::shared_ptr<GraphicsAbstraction::Buffer>& buffer) override;

		void GetData(void* data, uint32_t size, uint32_t offset) override;
		inline uint32_t GetSize() const override { return Size; }
	private:
		void UpdateDescriptor();
	private:
		VulkanContextReference m_Context;
	};

}