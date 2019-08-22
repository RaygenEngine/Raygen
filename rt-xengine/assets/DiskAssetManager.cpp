#include "pch.h"
#include "DiskAssetManager.h"

namespace Assets
{
	DiskAssetManager::DiskAssetManager(System::Engine* context)
		: EngineObject(context)
	{
		RT_XENGINE_LOG_INFO("Initializing File manager, id: {}", GetObjectId());
	}

	std::shared_ptr<Texture> DiskAssetManager::LoadTextureAsset(const std::string& texturePath, DYNAMIC_RANGE dr, bool flipVertically,
		const std::string& pathHint)
	{
		auto path = m_pathSystem.SearchAsset(texturePath, pathHint);

		if (path.empty())
			return nullptr;

		return LoadAssetAtMultiKeyCache<Texture>(m_textures, this, path, dr, flipVertically);
	}

	std::shared_ptr<CubeMap> DiskAssetManager::LoadCubeMapAsset(const std::string& texturePath, DYNAMIC_RANGE dr, bool flipVertically, const std::string& pathHint)
	{
		auto path = m_pathSystem.SearchAsset(texturePath, pathHint);

		if (path.empty())
			return nullptr;

		return LoadAssetAtMultiKeyCache<CubeMap>(m_cubeMaps, this, path, dr, flipVertically);
	}

	std::shared_ptr<PackedTexture> DiskAssetManager::LoadPackedTexture(Texture* textTargetRChannel,
		uint32 actualComponents0, Texture* textTargetGChannel, uint32 actualComponents1, Texture* textTargetBChannel,
		uint32 actualComponents2, Texture* textTargetAChannel, uint32 actualComponents3, DYNAMIC_RANGE dr)
	{
		return LoadAssetAtMultiKeyCache<PackedTexture>(m_packedTextures, this, textTargetRChannel, actualComponents0, textTargetGChannel, actualComponents1,
			textTargetBChannel, actualComponents2, textTargetAChannel, actualComponents3, dr);
	}

	bool DiskAssetManager::Init(const std::string& applicationPath, const std::string& dataDirectoryName)
	{
		return m_pathSystem.Init(applicationPath, dataDirectoryName);
	}

	void DiskAssetManager::UnloadAssets()
	{
		auto Unload = [](auto files)
		{
			for (auto& asset : files)
			{
				auto ptr = asset.second.lock();
				if (ptr) ptr->Unload();
			}
		};

		Unload(m_files);
		Unload(m_textures);
		Unload(m_cubeMaps);
		Unload(m_packedTextures);
	}
}
