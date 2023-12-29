#pragma once

#include <memory>

#include <GraphicsAbstraction/Core/Log.h>
#include <GraphicsAbstraction/Core/Core.h>

namespace GraphicsAbstraction {

	enum class Filter
	{
		Nearest,
		Linear
	};

	class Sampler : public RefCounted
	{
	public:
		virtual ~Sampler() = default;

		virtual uint32_t GetHandle() const = 0;

		static Ref<Sampler> Create(Filter min, Filter mag);
	};

}