#include "Ringbuffer.h"

#include <Core/Core.h>

namespace GraphicsAbstraction {

	Ringbuffer::Ringbuffer(uint32_t size, const ImmediateFlushCallback& callback)
	{
		m_Size = size;
		m_ImmediateFlushCallback = callback;
		m_Buffer = Buffer::Create(size, BufferUsage::TransferSrc, BufferFlags::Mapped);
	}

	void Ringbuffer::Write(const void* src, Ref<Buffer> dst, uint32_t size, uint32_t dstOffset)
	{
		WriteInternal(src, size);
		m_BufferPackets.push_back({
			.SourceOffset = m_Offset,
			.DestinationOffset = dstOffset,
			.Size = size,
			.Destination = dst
		});

		IncreaseSize(size);
	}

	void Ringbuffer::Write(const void* src, Ref<Image> dst, uint32_t size)
	{
		WriteInternal(src, size);
		m_ImagePackets.push_back({
			.SourceOffset = m_Offset,
			.Size = size,
			.Destination = dst
		});

		IncreaseSize(size);
	}

	void Ringbuffer::Flush(Ref<CommandList>& cmd)
	{
		for (auto& packet : m_BufferPackets)
			cmd->CopyBufferRegion(m_Buffer, packet.Destination, packet.Size, packet.SourceOffset, packet.DestinationOffset);
		m_BufferPackets.clear();

		for (auto& packet : m_ImagePackets)
			cmd->CopyToImage(m_Buffer, packet.Destination, packet.SourceOffset);
		m_ImagePackets.clear();

		m_WrittenSize = 0;
	}

	void Ringbuffer::WriteInternal(const void* src, uint32_t size)
	{
		if ((m_WrittenSize + size) > m_Size) m_ImmediateFlushCallback();

		const uint32_t part1 = std::min(size, m_Size - m_Offset);
		const uint32_t part2 = size - part1;

		m_Buffer->SetData(src, part1, m_Offset);
		if (part2) m_Buffer->SetData((char*)src + part1, part2);
	}

	void Ringbuffer::IncreaseSize(uint32_t size)
	{
		m_Offset = (m_Offset + size) % m_Size;
		m_WrittenSize += size;
		GA_CORE_ASSERT(m_WrittenSize < m_Size);
	}

}