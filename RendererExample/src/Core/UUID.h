#pragma once

#include <cstdint>
#include <unordered_map>

namespace GraphicsAbstraction {

	class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		operator uint64_t() const { return m_UUID; }
	private:
		uint64_t m_UUID;
	};

}

namespace std {

	template<>
	struct hash<GraphicsAbstraction::UUID>
	{
		std::size_t operator()(const GraphicsAbstraction::UUID& uuid) const
		{
			return (uint64_t)uuid;
		}
	};

}