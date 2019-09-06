#include "pch.h"

#include "assets/AssetManager.h"

std::shared_ptr<StringFile> AssetManager::LoadStringFileAsset(const std::string& stringFilePath,
	const std::string& pathHint)
{
	const auto path = m_pathSystem.SearchAsset(stringFilePath, pathHint);

	if (path.empty())
		return nullptr;

	return CachingAux::LoadAssetAtMultiKeyCache<StringFile>(m_stringFiles, this, path, path);
}

std::shared_ptr<XMLDoc> AssetManager::LoadXMLDocAsset(const std::string& xmlDocPath,
	const std::string& pathHint)
{
	const auto path = m_pathSystem.SearchAsset(xmlDocPath, pathHint);

	if (path.empty())
		return nullptr;

	return CachingAux::LoadAssetAtMultiKeyCache<XMLDoc>(m_xmlDocs, this, path, path);
}

std::shared_ptr<Texture> AssetManager::LoadTextureAsset(const std::string& texturePath, DynamicRange dr, bool flipVertically,
	const std::string& pathHint)
{
	const auto path = m_pathSystem.SearchAsset(texturePath, pathHint);

	if (path.empty())
		return nullptr;

	return CachingAux::LoadAssetAtMultiKeyCache<Texture>(m_textures, this, path, path, dr, flipVertically);
}

std::shared_ptr<Model> AssetManager::LoadModelAsset(const std::string& modelPath, GeometryUsage usage,
	const std::string& pathHint)
{
	const auto path = m_pathSystem.SearchAsset(modelPath, pathHint);

	if (path.empty())
		return nullptr;

	return CachingAux::LoadAssetAtMultiKeyCache<Model>(m_models, this, path, path, usage);
}

std::shared_ptr<CubeMap> AssetManager::LoadCubeMapAsset(const std::string& texturePath, DynamicRange dr, bool flipVertically, const std::string& pathHint)
{
	const auto path = m_pathSystem.SearchAsset(texturePath, pathHint);

	if (path.empty())
		return nullptr;

	return CachingAux::LoadAssetAtMultiKeyCache<CubeMap>(m_cubeMaps, this, path, path, dr, flipVertically);
}

std::shared_ptr<PackedTexture> AssetManager::LoadPackedTexture(Texture* textTargetRChannel,
	uint32 actualComponents0, Texture* textTargetGChannel, uint32 actualComponents1, Texture* textTargetBChannel,
	uint32 actualComponents2, Texture* textTargetAChannel, uint32 actualComponents3, DynamicRange dr)
{
	//return LoadAssetAtMultiKeyCache<PackedTexture>(m_packedTextures, this, textTargetRChannel, actualComponents0, textTargetGChannel, actualComponents1,
	//	textTargetBChannel, actualComponents2, textTargetAChannel, actualComponents3, dr);
	return nullptr;
}

bool AssetManager::Init(const std::string& applicationPath, const std::string& dataDirectoryName)
{
	return m_pathSystem.Init(applicationPath, dataDirectoryName);
}

void AssetManager::UnloadAssets()
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
