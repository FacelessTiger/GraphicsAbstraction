#pragma once

namespace GraphicsAbstraction {

	class CommandBuffer
	{
	public:
		virtual ~CommandBuffer() = default;

		virtual void Reset() const = 0;

		virtual void Begin() const = 0;
		virtual void End() const = 0;
	};

}