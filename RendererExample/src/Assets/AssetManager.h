#pragma once

#include <GraphicsAbstraction/GraphicsAbstraction.h>
#include <Assets/Asset.h>

#include <unordered_map>

namespace GraphicsAbstraction {

	class AssetManager
	{
	public:
		static Ref<Asset> GetAsset(AssetHandle handle) { return m_AssetMap[handle]; }
		static AssetHandle AddAsset(const Ref<Asset>& asset);

		static const std::unordered_map<AssetHandle, Ref<Asset>>& GetMap() { return m_AssetMap; }
	private:
		static std::unordered_map<AssetHandle, Ref<Asset>> m_AssetMap;
	};

}