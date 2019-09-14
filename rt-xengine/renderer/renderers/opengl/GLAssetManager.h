#pragma once

#include "asset/Asset.h"
#include "asset/AssetPod.h"
#include "renderer/renderers/opengl/GLAsset.h"

namespace OpenGL
{
	class GLAssetManager
	{		
		std::unordered_map<std::string, GLAsset*> m_assetMap;

	public:

		bool Load(GLAsset* asset)
		{
			assert(asset);
			assert(m_assetMap.find(asset->m_assetManagerPodPath.string()) != m_assetMap.end());

			if (asset->m_isLoaded)
			{
				return true;
			}

			if (asset->FriendLoad())
			{
				asset->m_isLoaded = true;
			}
			else
			{
				asset->m_isLoaded = false;
			}
			
			return asset->m_isLoaded;
		}

		template<typename AssetT>
		AssetT* RequestLoadAsset(const fs::path& assetManagerPodPath)
		{
			auto it = m_assetMap.find(assetManagerPodPath.string());
			if (it != m_assetMap.end())
			{
				auto* p = dynamic_cast<AssetT*>(it->second);
				assert(p);
				return p;
			}
			AssetT* result = new AssetT(assetManagerPodPath);
			m_assetMap.emplace(assetManagerPodPath.string(), result);

			assert(Load(result));
			
			return result;
		}

		void Unload(GLAsset* asset)
		{
			if (asset->m_isLoaded)
			{
				asset->Unload();
			}
			asset->m_isLoaded = false;
		}
	};

}
