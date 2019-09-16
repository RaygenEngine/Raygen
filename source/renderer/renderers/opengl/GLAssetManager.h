#pragma once

#include "renderer/renderers/opengl/GLAsset.h"

namespace OpenGL
{
	class GLAssetManager
	{		
		std::unordered_map<uint64, GLAsset*> m_assetMap;

	public:
		// This will also instantly load the GPU asset.
		// constructorArguments are forwarded to the AssetT constructor ONLY IF the element is created.
		template<typename AssetT, typename ...Args>
		AssetT* GetOrMake(uint64 cacheHash, Args&& ...constructorArguments)
		{
			static_assert(std::is_base_of_v<GLAsset, AssetT>, "Expected a child of GLAsset.");
			auto it = m_assetMap.find(cacheHash);
			if (it != m_assetMap.end())
			{
				return dynamic_cast<AssetT*>(it->second);
			}
			AssetT* result = new AssetT(std::forward<Args>(constructorArguments)...);
			m_assetMap.emplace(cacheHash, result);
			result->FriendLoad();
			return result;
		}

		// This will also instantly load the GPU asset.
		// constructorArguments are forwarded to the AssetT constructor ONLY IF the element is created.
		template<typename AssetT, typename PtrT, typename ...Args>
		AssetT* GetOrMake(PtrT* cacheHash, Args&& ...constructorArguments)
		{
			assert(pointer != nullptr);
			return GetOrMake<AssetT>(reinterpret_cast<uint64>(cacheHash), std::forward<Args>(constructorArguments)...);
		}
		
		// For ease of use, this will forward the pointer as a single constructor argument
		// and at the same time cache with this pointer as hash.
		template<typename AssetT, typename PtrT>
		AssetT* GetOrMakeFromPtr(PtrT* pointer)
		{
			assert(pointer != nullptr);
			return GetOrMake<AssetT>(reinterpret_cast<uint64>(pointer), pointer);
		}

		template<typename AssetT>
		AssetT* GetOrMakeFromUri(const fs::path& path)
		{
			std::hash<std::string> hasher;

			uint64 hashValue = static_cast<uint64>(hasher(path.string()));

			return GetOrMake<AssetT>(hashValue, path);
		}


		// This will delete the element at that hash if found.
		void Delete(uint64 cacheHash);

			// This will delete the element at that hash if found.
		template<typename PtrT>
		void Delete(PtrT* cacheHash)
		{
			Delete(static_cast<uint64>(cacheHash));
		}
	};

}
