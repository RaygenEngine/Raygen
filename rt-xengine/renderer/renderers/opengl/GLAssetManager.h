#pragma once

#include "asset/Asset.h"

namespace OpenGL
{
	class GLAsset;
	
	class GLAssetManager
	{		
		std::unordered_map<void*, GLAsset*> m_assetMap;

	public:

		//template<typename AssetT>
		//bool Load(AssetT* asset)
		//{
		//	assert(asset);
		//	assert(m_assetMap.find(asset->m_asset) != m_assetMap.end());

		//	if (asset->m_isLoaded)
		//	{
		//		return true;
		//	}

		//	if (asset->FriendLoad())
		//	{
		//		asset->m_isLoaded = true;
		//	}
		//	return asset->m_isLoaded;
		//}

		//// If this returns null, an asset of a different type already exists at this key asset
		////template<typename AssetT>
		////AssetT* MaybeGenerateAsset(void* keyAsset)
		////{
		////	auto it = m_assetMap.find(keyAsset);
		////	if (it != m_assetMap.end())
		////	{
		////		return dynamic_cast<AssetT*>(it->second);
		////	}
		////	AssetT* result = new AssetT(keyAsset);
		////	m_assetMap.emplace(keyAsset, result);
		////	return result;
		////}


		//// todo:
		//template<typename AssetT>
		//void Unload(AssetT* asset)
		//{
		//	asset->m_isLoaded = false;
		//	//assert(false);
		//}
	};

}
