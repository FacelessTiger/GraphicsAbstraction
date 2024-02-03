#pragma once

#include <Core/UUID.h>
#include <GraphicsAbstraction/GraphicsAbstraction.h>

#include <cstdint>
#include <string_view>

namespace GraphicsAbstraction {

	using AssetHandle = UUID;

	enum class AssetType : uint16_t
	{
		None = 0,
		Mesh,
		Material,
		Texture2D
	};

	std::string_view AssetTypeToString(AssetType type);
	AssetType AssetTypeFromString(std::string_view type);

	class Asset : public RefCounted
	{
	public:
		AssetHandle Handle;
		std::string Name;
	public:
		virtual ~Asset() = default;
		virtual AssetType GetType() const = 0;
	};

}