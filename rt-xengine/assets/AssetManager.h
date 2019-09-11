#pragma once

#include "system/EngineComponent.h"
#include "system/reflection/Reflector.h"
#include "system/Engine.h"
#include "assets/PathSystem.h"
#include "assets/Asset.h"

// asset cache responsible for "cpu" files (xmd, images, string files, xml files, etc)
class AssetManager
{
	std::unordered_map<std::string, Asset*> m_assetMap;

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
	AssetT* MaybeGenerateAsset(const fs::path& path)
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
		assert(false);
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

//class TextureAsset : public Asset
//{
//public:
//	TextureAsset(fs::path uri)
//		: Asset(uri) {}
//
//	std::vector<byte> buffer;
//
//private:
//	bool Load() override 
//	{
//		buffer = std::vector<byte>({ 1,2,3 });
//	}
//
//	void Unload() override 
//	{
//		buffer.swap(std::vector<byte>());
// 	}
//};
//
//class CubemapAsset : public Asset
//{
//public:
//	CubemapAsset(fs::path uri)
//		: Asset(uri) {}
//
//	TextureAsset* m_sides[6];
//
//private:
//	bool Load() override
//	{
//		fs::path without_ext = m_uri.filename();
//
//		for (int32 i = 0; i < 6; i++)
//		{
//			fs::path thisfile = without_ext;
//
//			thisfile += std::vector({
//				"RIGHT",
//				"LEFT",
//				"UP",
//				"DOWN",
//				"FRONT",
//				"BACK"
//			})[i];
//
//			thisfile += m_uri.extension();
//
//			m_sides[i] = GetAM()->MaybeGenerateAsset<TextureAsset>(thisfile);
//			GetAM()->Load(m_sides[i]);
//		}
//	}
//
//};
