#pragma once

#include <optional>

namespace GraphicsAbstraction {

	class D3D12ResourceHandle
	{
	public:
		D3D12ResourceHandle() = default;
		~D3D12ResourceHandle();

		uint32_t GetValue() const;
	private:
		mutable std::optional<uint32_t> m_ID;
	};

}