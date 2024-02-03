#include "Asset.h"

namespace GraphicsAbstraction {

    std::string_view AssetTypeToString(AssetType type)
    {
        switch (type)
        {
            case AssetType::None:       return "AssetType::None";
            case AssetType::Mesh:       return "AssetType::Mesh";
            case AssetType::Material:   return "AssetType::Material";
            case AssetType::Texture2D:  return "AssetType::Texture2D";
        }

        return "AssetType::<Invalid>";
    }

    AssetType AssetTypeFromString(std::string_view type)
    {
        if (type == "AssetType::None")      return AssetType::None;
        if (type == "AssetType::Mesh")      return AssetType::Mesh;
        if (type == "AssetType::Material")  return AssetType::Material;
        if (type == "AssetType::Texture2D") return AssetType::Texture2D;

        return AssetType::None;
    }

}