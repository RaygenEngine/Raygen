#pragma once

#include "system/EngineComponent.h"
#include "assets/PathSystem.h"
#include "assets/texture/Texture.h"
#include "assets/model/Model.h"
#include "assets/other/utf8/StringFile.h"
#include "assets/other/xml/XMLDoc.h"
#include "assets/CachingAux.h"

// asset cache responsible for "cpu" files (xmd, images, string files, xml files, etc)
class AssetManager
{
	PathSystem m_pathSystem;

	CachingAux::MultiKeyAssetCache<StringFile, std::string> m_stringFiles;
	CachingAux::MultiKeyAssetCache<XMLDoc, std::string> m_xmlDocs;
	CachingAux::MultiKeyAssetCache<Model, std::string, GeometryUsage> m_models;
	CachingAux::MultiKeyAssetCache<Texture, std::string> m_textures;

public:
	std::shared_ptr<StringFile> LoadStringFileAsset(const std::string& stringFilePath, const std::string& pathHint = "");
	std::shared_ptr<XMLDoc> LoadXMLDocAsset(const std::string& xmlDocPath, const std::string& pathHint = "");
	std::shared_ptr<Model> LoadModelAsset(const std::string& modelPath, GeometryUsage usage = GeometryUsage::STATIC, const std::string& pathHint = "");
	std::shared_ptr<Texture> LoadTextureAsset(const std::string& texturePath, const std::string& pathHint = "");

	bool Init(const std::string& applicationPath, const std::string& dataDirectoryName);

	void UnloadAssets();
};
