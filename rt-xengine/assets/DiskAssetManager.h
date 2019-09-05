#pragma once

#include "system/EngineObject.h"
#include "assets/PathSystem.h"
#include "assets/texture/Texture.h"
#include "assets/texture/CubeMap.h"
#include "assets/texture/PackedTexture.h"
#include "assets/model/Model.h"
#include "assets/other/utf8/StringFile.h"
#include "assets/other/xml/XMLDoc.h"
#include "assets/CachingAux.h"

namespace Assets
{
	// asset cache responsible for "cpu" files (xmd, images, string files, xml files, etc)
	class DiskAssetManager : public System::EngineObject
	{
		PathSystem m_pathSystem;

		MultiKeyAssetCache<StringFile, std::string> m_stringFiles;
		MultiKeyAssetCache<XMLDoc, std::string> m_xmlDocs;
		MultiKeyAssetCache<Model, std::string, GeometryUsage> m_models;
		MultiKeyAssetCache<Texture, std::string, DynamicRange, bool> m_textures;
		MultiKeyAssetCache<CubeMap, std::string, DynamicRange, bool> m_cubeMaps;
		MultiKeyAssetCache<PackedTexture, Texture*, uint32, Texture*, uint32, Texture*, uint32, Texture*, uint32, DynamicRange> m_packedTextures;

	public:
		DiskAssetManager(System::Engine* engine);

		std::shared_ptr<StringFile> LoadStringFileAsset(const std::string& stringFilePath, const std::string& pathHint = "");
		std::shared_ptr<XMLDoc> LoadXMLDocAsset(const std::string& xmlDocPath, const std::string& pathHint = "");
		std::shared_ptr<Model> LoadModelAsset(const std::string& modelPath, GeometryUsage usage = GeometryUsage::STATIC, const std::string& pathHint = "");
		std::shared_ptr<Texture> LoadTextureAsset(const std::string& texturePath, DynamicRange dr, bool flipVertically = true, const std::string& pathHint = "");
		std::shared_ptr<CubeMap> LoadCubeMapAsset(const std::string& texturePath, DynamicRange dr, bool flipVertically = true, const std::string& pathHint = "");
		std::shared_ptr<PackedTexture> LoadPackedTexture(Texture* textTargetRChannel, uint32 actualComponents0,
			                                             Texture* textTargetGChannel, uint32 actualComponents1,
			                                             Texture* textTargetBChannel, uint32 actualComponents2,
			                                             Texture* textTargetAChannel, uint32 actualComponents3, DynamicRange dr);

		bool Init(const std::string& applicationPath, const std::string& dataDirectoryName);

		void UnloadAssets();

		void ToString(std::ostream& os) const override { os << "object-type: DiskAssetManager, id: " << GetUID(); }
	};
}
