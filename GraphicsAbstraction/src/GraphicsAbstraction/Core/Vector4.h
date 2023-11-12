#pragma once

namespace GraphicsAbstraction {

	struct Vector4
	{
		float x, y, z, w;

		Vector4(float x, float y, float z, float w)
			: x(x), y(y), z(z), w(w)
		{ }
	};

}