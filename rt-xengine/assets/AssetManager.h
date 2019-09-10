#pragma once

#include "system/EngineComponent.h"
#include "assets/PathSystem.h"
#include "assets/texture/Texture.h"
#include "assets/model/Model.h"
#include "assets/other/utf8/StringFile.h"
#include "assets/other/xml/XMLDoc.h"
#include "assets/CachingAux.h"

#include <filesystem>
#include "system/reflection/Reflector.h"
#include "system/Engine.h"


namespace fs = std::filesystem;

class Asset {
protected:
	Asset(fs::path uri)
		: m_uri(uri)
	{
		assert(!uri.string().empty());
	}
	
	fs::path m_uri;
	bool m_isLoaded{ false };

	virtual bool Load() = 0;
	virtual void Unload() = 0;
private:
	bool FriendLoad()   { return Load(); }
	void FriendUnload() { Unload(); }

	friend class AssetManager;
};

// asset cache responsible for "cpu" files (xmd, images, string files, xml files, etc)
class AssetManager
{
private:
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

private:
	PathSystem m_pathSystem;

	CachingAux::MultiKeyAssetCache<StringFile, std::string> m_stringFiles;
	CachingAux::MultiKeyAssetCache<XMLDoc, std::string> m_xmlDocs;
	CachingAux::MultiKeyAssetCache<Model, std::string, GeometryUsage> m_models;
	CachingAux::MultiKeyAssetCache<Texture, std::string> m_textures;

public:
	std::shared_ptr<StringFile> LoadStringFileAsset(const std::string& stringFilePath, const std::string& pathHint = "");
	std::shared_ptr<XMLDoc> LoadXMLDocAsset(const std::string& xmlDocPath, const std::string& pathHint = "");
	std::shared_ptr<Model> LoadModelAsset(const std::string& modelPath, GeometryUsage usage = GeometryUsage::STATIC, const std::string& pathHint = "");
	std::shared_ptr<Texture> LoadTextureAsset(const std::string& texturePath, const std::string& pathHint = "");

	bool Init(const std::string& applicationPath, const std::string& dataDirectoryName);
	void UnloadAssets();
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
