#include "pch.h"

#include "assets/DiskAssetManager.h"

namespace Assets
{
	DiskAssetManager::DiskAssetManager(System::Engine* engine)
		: EngineObject(engine)
	{
	}

	std::shared_ptr<StringFile> DiskAssetManager::LoadStringFileAsset(const std::string& stringFilePath,
		const std::string& pathHint)
	{
		const auto path = m_pathSystem.SearchAsset(stringFilePath, pathHint);

		if (path.empty())
			return nullptr;

		return Assets::LoadAssetAtMultiKeyCache<StringFile>(m_stringFiles, this, path, path);
	}

	std::shared_ptr<XMLDoc> DiskAssetManager::LoadXMLDocAsset(const std::string& xmlDocPath,
		const std::string& pathHint)
	{
		const auto path = m_pathSystem.SearchAsset(xmlDocPath, pathHint);

		if (path.empty())
			return nullptr;

		return Assets::LoadAssetAtMultiKeyCache<XMLDoc>(m_xmlDocs, this, path, path);
	}

	std::shared_ptr<Texture> DiskAssetManager::LoadTextureAsset(const std::string& texturePath, DynamicRange dr, bool flipVertically,
		const std::string& pathHint)
	{
		const auto path = m_pathSystem.SearchAsset(texturePath, pathHint);

		if (path.empty())
			return nullptr;

		return Assets::LoadAssetAtMultiKeyCache<Texture>(m_textures, this, path, path, dr, flipVertically);
	}

	std::shared_ptr<Model> DiskAssetManager::LoadModelAsset(const std::string& modelPath, GeometryUsage usage,
		const std::string& pathHint)
	{
		const auto path = m_pathSystem.SearchAsset(modelPath, pathHint);

		if (path.empty())
			return nullptr;

		return Assets::LoadAssetAtMultiKeyCache<Model>(m_models, this, path, path, usage);
	}

	std::shared_ptr<CubeMap> DiskAssetManager::LoadCubeMapAsset(const std::string& texturePath, DynamicRange dr, bool flipVertically, const std::string& pathHint)
	{
		const auto path = m_pathSystem.SearchAsset(texturePath, pathHint);

		if (path.empty())
			return nullptr;

		return Assets::LoadAssetAtMultiKeyCache<CubeMap>(m_cubeMaps, this, path, path, dr, flipVertically);
	}

	std::shared_ptr<PackedTexture> DiskAssetManager::LoadPackedTexture(Texture* textTargetRChannel,
		uint32 actualComponents0, Texture* textTargetGChannel, uint32 actualComponents1, Texture* textTargetBChannel,
		uint32 actualComponents2, Texture* textTargetAChannel, uint32 actualComponents3, DynamicRange dr)
	{
		//return LoadAssetAtMultiKeyCache<PackedTexture>(m_packedTextures, this, textTargetRChannel, actualComponents0, textTargetGChannel, actualComponents1,
		//	textTargetBChannel, actualComponents2, textTargetAChannel, actualComponents3, dr);
		return nullptr;
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

		Unload(m_models);
		Unload(m_stringFiles);
		Unload(m_xmlDocs);
		Unload(m_textures);
		Unload(m_cubeMaps);
		Unload(m_packedTextures);
	}
}
