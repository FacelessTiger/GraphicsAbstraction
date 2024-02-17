#pragma once

#include <Assets/Asset.h>

namespace GraphicsAbstraction {

	class Material : public Asset
	{
	public:
		UUID RenderHandle;
		glm::vec4 AlbedoFactor;
		AssetHandle AlbedoTexture;
		AssetHandle MetallicRoughnessTexture;
		float MetallicFactor;
		float RoughnessFactor;
		float AO;
	public:
		virtual ~Material() = default;

		virtual AssetType GetType() const { return AssetType::Material; }
	};

}