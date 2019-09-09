#include "pch.h"

#include "assets/AssetManager.h"

std::shared_ptr<StringFile> AssetManager::LoadStringFileAsset(const std::string& stringFilePath,
	const std::string& pathHint)
{
	const auto path = m_pathSystem.SearchAsset(stringFilePath, pathHint);

	if (path.empty())
		return nullptr;

	return CachingAux::LoadAssetAtMultiKeyCache<StringFile>(m_stringFiles, path, path);
}

std::shared_ptr<XMLDoc> AssetManager::LoadXMLDocAsset(const std::string& xmlDocPath,
	const std::string& pathHint)
{
	const auto path = m_pathSystem.SearchAsset(xmlDocPath, pathHint);

	if (path.empty())
		return nullptr;

	return CachingAux::LoadAssetAtMultiKeyCache<XMLDoc>(m_xmlDocs, path, path);
}

std::shared_ptr<Texture> AssetManager::LoadTextureAsset(const std::string& texturePath,
	const std::string& pathHint)
{
	const auto path = m_pathSystem.SearchAsset(texturePath, pathHint);

	if (path.empty())
		return nullptr;

	return CachingAux::LoadAssetAtMultiKeyCache<Texture>(m_textures, path, path);
}

std::shared_ptr<Model> AssetManager::LoadModelAsset(const std::string& modelPath, GeometryUsage usage,
	const std::string& pathHint)
{
	const auto path = m_pathSystem.SearchAsset(modelPath, pathHint);

	if (path.empty())
		return nullptr;

	return CachingAux::LoadAssetAtMultiKeyCache<Model>(m_models, path, path, usage);
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
}
