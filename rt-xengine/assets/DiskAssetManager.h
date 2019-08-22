#ifndef DISKASSETMANAGER_H
#define DISKASSETMANAGER_H

#include "system/EngineObject.h"
#include "PathSystem.h"
#include "DiskAsset.h"
#include "texture/Texture.h"

#include "MultiKeyAssetCacheHashing.h"
#include "texture/CubeMap.h"
#include "texture/PackedTexture.h"

namespace Assets
{
	// asset cache responsible for "cpu" files (xmd, images, string files, xml files, etc)
	class DiskAssetManager : public System::EngineObject
	{
		PathSystem m_pathSystem;

		// Files that only require path as key
		MultiKeyAssetCache<DiskAsset, std::string> m_files;
		// Files that require path and texel type as key
		MultiKeyAssetCache<Texture, std::string, DYNAMIC_RANGE, bool> m_textures;
		// Files that require path and texel type as key
		MultiKeyAssetCache<CubeMap, std::string, DYNAMIC_RANGE, bool> m_cubeMaps;

		MultiKeyAssetCache<PackedTexture, Texture*, uint32, Texture*, uint32, Texture*, uint32, Texture*, uint32, DYNAMIC_RANGE> m_packedTextures;


	public:
		DiskAssetManager(System::Engine* context);
		~DiskAssetManager() = default;

		// AssetType must derive from FileAsset (note : use LoadTextureAsset to load textures)
		template <typename AssetType>
		std::shared_ptr<AssetType> LoadFileAsset(const std::string& assetPath, const std::string& pathHint = "")
		{
			auto path = m_pathSystem.SearchAsset(assetPath, pathHint);

			if (path.empty())
				return nullptr;

			return LoadAssetAtMultiKeyCache<AssetType>(m_files, this, path);
		}

		// OpenGL, Optix and many more require textures vertically flipped, if your api doesn't then flipVertically should be set to false
		std::shared_ptr<Texture> LoadTextureAsset(const std::string& texturePath, DYNAMIC_RANGE dr, bool flipVertically = true, const std::string& pathHint = "");

		std::shared_ptr<CubeMap> LoadCubeMapAsset(const std::string& texturePath, DYNAMIC_RANGE dr, bool flipVertically = true, const std::string& pathHint = "");

		std::shared_ptr<PackedTexture> LoadPackedTexture(Texture* textTargetRChannel, uint32 actualComponents0,
			                                             Texture* textTargetGChannel, uint32 actualComponents1,
			                                             Texture* textTargetBChannel, uint32 actualComponents2,
			                                             Texture* textTargetAChannel, uint32 actualComponents3, DYNAMIC_RANGE dr);

		bool Init(const std::string& applicationPath, const std::string& dataDirectoryName);

		void UnloadAssets();
	};

}

#endif // DISKASSETMANAGER_H
