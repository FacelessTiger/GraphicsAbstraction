#pragma once

#include <GraphicsAbstraction/GraphicsAbstraction.h>
#include <Assets/Asset.h>

namespace GraphicsAbstraction {

	struct Primitive
	{
		AssetHandle Material;

		Primitive(AssetHandle material)
			: Material(material)
		{ }
	};

	class Mesh : public Asset
	{
	public:
		std::vector<Primitive> Primitives;
		UUID RenderHandle;
	public:
		virtual ~Mesh() = default;

		virtual AssetType GetType() const { return AssetType::Mesh; }
	};

}