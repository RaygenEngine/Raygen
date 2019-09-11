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
public:
	AssetReflector m_reflector;
	[[nodiscard]] fs::path GetUri() const { return m_uri; }
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

inline AssetReflector& GetReflector(Asset* object)
{
	return object->m_reflector;
}

class BackgroundColorAsset : public Asset
{
public:
	BackgroundColorAsset(fs::path uri)
		: Asset(uri) 
	{
		REFLECT_VAR(m_color, PropertyFlags::Color);
		REFLECT_VAR(m_self);
	}
	glm::vec3 m_color;
	BackgroundColorAsset* m_self;

private:
	bool Load() override 
	{
		m_self = this;
		m_color = { (std::rand() % 6) * 0.1f ,  (std::rand() % 6) * 0.1f, 0.4 };
		return true;
	}

	void Unload() override 
	{

	}
};