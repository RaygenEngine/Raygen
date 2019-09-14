#pragma once

#include "system/EngineComponent.h"
#include "system/reflection/Reflector.h"
#include "system/Engine.h"
#include "asset/PathSystem.h"
#include "asset/Asset.h"

constexpr auto __default__imageWhite = "__default__image-white.jpg";
constexpr auto __default__imageMissing = "__default__image-missing.jpg";

constexpr auto __default__texture = "#__default__texture";
constexpr auto __default__material = "#__default__material";

// asset cache responsible for "cpu" files (xmd, images, string files, xml files, etc)
class AssetManager
{
	std::unordered_map<std::string, Asset*> m_pathAssetMap;
	std::unordered_map<AssetPod*, Asset*> m_podAssetMap;
	friend class Editor;
public:

	bool Load(Asset* asset)
	{
		assert(asset);
		assert(m_pathAssetMap.find(asset->m_uri.string()) != m_pathAssetMap.end());

		if (asset->m_isLoaded)
		{
			return true;
		}
		
		asset->m_isLoaded = asset->FriendLoad();
		
		return asset->m_isLoaded;
	}

	// loads the parent asset of a pod (refreshes the pod's data)
	template<typename PodType>
	PodType* RequestFreshPod(const fs::path& podAssetPath)
	{		
		assert(m_pathAssetMap.find(podAssetPath.string()) != m_pathAssetMap.end());
		
		// get assoc asset
		auto asset = dynamic_cast<PodedAsset<PodType>*>(m_pathAssetMap[podAssetPath.string()]);
		assert(asset);
		assert(Load(asset));

		// (re)load it
		return dynamic_cast<PodType*>(asset->m_pod);
	}

	void RefreshPod(AssetPod* pod)
	{
		assert(m_podAssetMap.find(pod) != m_podAssetMap.end());

		// get assoc asset
		const auto asset = m_podAssetMap[pod];
		assert(Load(asset));
	}

	fs::path GetPodPath(AssetPod* pod)
	{
		assert(m_podAssetMap.find(pod) != m_podAssetMap.end());

		// get assoc asset
		const auto asset = m_podAssetMap[pod];
		return asset->GetUri();
	}

	template<typename AssetT>
	AssetT* RequestSearchAsset(const fs::path& path)
	{
		// PERF:
		fs::path p;
		if (IsCpuPath(path))
		{
			p = path;
		}
		else 
		{
			// PERF:
			p = m_pathSystem.SearchAssetPath(path);
			assert(!p.empty());
		}

		auto it = m_pathAssetMap.find(p.string());
		if (it != m_pathAssetMap.end())
		{
			auto* asset = dynamic_cast<AssetT*>(it->second);
			assert(asset);
			return asset;
		}
		
		AssetT* asset = new AssetT(p);
		m_pathAssetMap.emplace(p.string(), asset);

		asset->Allocate();
		m_podAssetMap[asset->m_pod] = asset;
		
		return asset;
	}

	void Unload(Asset* asset)
	{
		if(asset->m_isLoaded)
		{
			asset->Deallocate();
		}
		
		asset->m_isLoaded = false;
	}

	static bool IsCpuPath(const fs::path& path)
	{
		if (path.filename().string()[0] == '#') 
			return true;
		return false;
	}

	PathSystem m_pathSystem;
	bool Init(const std::string& applicationPath, const std::string& dataDirectoryName);
};
