#include "AssetManager.h"

namespace GraphicsAbstraction {

	std::unordered_map<AssetHandle, Ref<Asset>> AssetManager::m_AssetMap;

	AssetHandle AssetManager::AddAsset(const Ref<Asset>& asset)
	{
		AssetHandle handle;

		m_AssetMap[handle] = asset;
		return handle;
	}

}