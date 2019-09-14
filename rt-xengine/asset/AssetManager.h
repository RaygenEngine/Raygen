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
		Load(asset);
		// (re)load it
		return dynamic_cast<PodType*>(asset->m_pod);
	}

	void RefreshPod(AssetPod* pod)
	{
		assert(m_podAssetMap.find(pod) != m_podAssetMap.end());

		// get assoc asset
		const auto asset = m_podAssetMap[pod];
		Load(asset);
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

	void Unload(Asset* asset);
	
	void UnloadAll()
	{
		for (auto& p : m_pathAssetMap) 
		{
			Unload(p.second);
		}
	}


	static bool IsCpuPath(const fs::path& path)
	{
		if (path.filename().string()[0] == '#') 
			return true;
		return false;
	}

	//template<typename ...Args>
	//bool LoadAssetList(Args... args)
	//{
	//	using namespace std;
	//	//static_assert(conjunction_v< (conjunction_v < is_pointer_v<Args>, is_base_of_v<std::remove_pointer_t<Args>, Asset>), ... >, "Not all argument types are pointers of assets.");

	//	return ((Load(args) && ...));
	//}

	////template<typename AssetT>
	////AssetT* GenerateAndLoad(const fs::path& path)
	////{
	////	auto r = GenerateAsset(path);
	////	Load(r);
	////	return r;
	////}
	PathSystem m_pathSystem;
	bool Init(const std::string& applicationPath, const std::string& dataDirectoryName);
};
