#pragma once

#include <GraphicsAbstraction/Renderer/Buffer.h>

namespace GraphicsAbstraction {

	class D3D12Buffer : public Buffer
	{
	public:
		D3D12Buffer(uint32_t size, BufferUsage usage, BufferFlags flags) { }

		void SetData(const void* data, uint32_t size = 0, uint32_t offset = 0) override { }
		void SetData(const Ref<Buffer>& buffer) override { }

		void GetData(void* data, uint32_t size, uint32_t offset) override { }
		uint32_t GetHandle() const override { return 0; }
		uint32_t GetSize() const override { return 0; }
	};

}