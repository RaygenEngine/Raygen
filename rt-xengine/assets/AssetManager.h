#pragma once

#include "system/EngineComponent.h"
#include "system/reflection/Reflector.h"
#include "system/Engine.h"
#include "assets/PathSystem.h"
#include "assets/Asset.h"

constexpr auto __default__textureWhite = "__default__texture-white.jpg";
constexpr auto __default__textureMissing = "__default__texture-missing.jpg";

// asset cache responsible for "cpu" files (xmd, images, string files, xml files, etc)
class AssetManager
{
	std::unordered_map<std::string, Asset*> m_assetMap;
	friend class Editor;
public:
	template<typename AssetT>
	bool Load(AssetT* asset)
	{
		assert(asset);
		assert(m_assetMap.find(asset->m_uri.string()) != m_assetMap.end());

		if (asset->m_isLoaded)
		{
			return true;
		}

		if (asset->FriendLoad())
		{
			asset->m_isLoaded = true;
		}
		return asset->m_isLoaded;
	}

	// If this returns null, an asset of a different type already exists at this uri
	template<typename AssetT>
	AssetT* RequestAsset(const fs::path& path)
	{
		auto it = m_assetMap.find(path.string());
		if (it != m_assetMap.end())
		{
			return dynamic_cast<AssetT*>(it->second);
		}
		AssetT* result = new AssetT(path);
		m_assetMap.emplace(path.string(), result);
		return result;
	}


	// todo:
	template<typename AssetT>
	void Unload(AssetT* asset)
	{
		asset->m_isLoaded = false;
		//assert(false);
	}


	//template<typename AssetT>
	//AssetT* GenerateAndLoad(const fs::path& path)
	//{
	//	auto r = GenerateAsset(path);
	//	Load(r);
	//	return r;
	//}
	PathSystem m_pathSystem;
	bool Init(const std::string& applicationPath, const std::string& dataDirectoryName);
};
