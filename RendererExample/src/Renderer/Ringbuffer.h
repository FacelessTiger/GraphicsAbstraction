#pragma once

#include <GraphicsAbstraction/GraphicsAbstraction.h>

namespace GraphicsAbstraction {

	struct RingBufferPacket
	{
		uint32_t SourceOffset, DestinationOffset, Size;
		Ref<Buffer> Destination;
	};

	struct RingImagePacket
	{
		uint32_t SourceOffset, Size;
		Ref<Image> Destination;
	};

	class Ringbuffer
	{
	public:
		using ImmediateFlushCallback = std::function<void()>;
		Ringbuffer(uint32_t size, const ImmediateFlushCallback& callback);

		void Write(const void* src, Ref<Buffer> dst, uint32_t size, uint32_t dstOffset);
		void Write(const void* src, Ref<Image> dst, uint32_t size);
		void Flush(Ref<CommandList>& cmd);
	private:
		void WriteInternal(const void* src, uint32_t size);
		void IncreaseSize(uint32_t size);
	private:
		ImmediateFlushCallback m_ImmediateFlushCallback;
		uint32_t m_Size;
		uint32_t m_Offset = 0, m_WrittenSize = 0;

		Ref<Buffer> m_Buffer;
		std::vector<RingBufferPacket> m_BufferPackets;
		std::vector<RingImagePacket> m_ImagePackets;
	};

}