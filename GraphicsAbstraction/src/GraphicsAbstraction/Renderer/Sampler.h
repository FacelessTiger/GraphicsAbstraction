#pragma once

#include <memory>

#include <GraphicsAbstraction/Core/Log.h>

namespace GraphicsAbstraction {

	enum class Filter
	{
		Nearest,
		Linear
	};

	class Sampler
	{
	public:
		virtual ~Sampler() = default;

		virtual uint32_t GetHandle() const = 0;

		static std::shared_ptr<Sampler> Create(Filter min, Filter mag);
	};

}