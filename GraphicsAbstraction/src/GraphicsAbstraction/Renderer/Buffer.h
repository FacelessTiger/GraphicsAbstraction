#pragma once

#include <GraphicsAbstraction/Core/Core.h>
#include <memory>

namespace GraphicsAbstraction {

	class GraphicsContext;

	enum class BufferUsage : uint32_t
	{
		StorageBuffer = 1,
		TransferSrc = 2,
		TransferDst = 4,
		IndexBuffer = 8,
		IndirectBuffer = 16
	};

	enum class BufferFlags : uint32_t
	{
		None = 0,
		Mapped = 1,
		DeviceLocal = 2
	};

	inline BufferUsage operator|(BufferUsage a, BufferUsage b) { return (BufferUsage)((int)a | (int)b); };
	inline bool operator&(BufferUsage a, BufferUsage b) { return (int)a & (int)b; };

	inline BufferFlags operator|(BufferFlags a, BufferFlags b) { return (BufferFlags)((int)a | (int)b); };
	inline bool operator&(BufferFlags a, BufferFlags b) { return (int)a & (int)b; };

	class Buffer : public RefCounted
	{
	public:
		virtual ~Buffer() = default;

		virtual void SetData(const void* data, uint32_t size = 0, uint32_t offset = 0) = 0;
		virtual void SetData(const Ref<Buffer>& buffer) = 0;

		virtual void GetData(void* data, uint32_t size, uint32_t offset) = 0;
		virtual uint32_t GetHandle() const = 0;
		virtual uint32_t GetSize() const = 0;

		static Ref<Buffer> Create(uint32_t size, BufferUsage usage, BufferFlags flags = BufferFlags::None);
	};

}