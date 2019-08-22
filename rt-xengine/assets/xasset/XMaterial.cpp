#include "pch.h"
#include "XMaterial.h"

#include "assets/other/xml/ParsingAux.h"


namespace Assets
{
	// default material values
	XMaterial::XMaterial(DiskAsset* parent)
		: DiskAssetPart(parent)
	{
	}

	template <typename DefaultValueType>
	std::shared_ptr<Texture> XMaterial::LoadMap(const std::string& path, const std::string& type, const tinyxml2::XMLElement* materialData, DYNAMIC_RANGE dr, bool loadDefaultIfMissing)
	{
		// load map at path
		std::shared_ptr<Texture> map;
		//  load map at path
		if (!path.empty())
			map = GetDiskAssetManager()->LoadTextureAsset(path, dr);

		// if missing create default
		if (!map && loadDefaultIfMissing)
		{
			DefaultValueType defaultVal{};
			const auto components = sizeof(DefaultValueType) / sizeof(float);
			ReadFloatsAttribute(materialData, "default", defaultVal);

			switch (dr)
			{
			case DR_LOW:
				byte cDefVal[components];

				for (auto i = 0u; i < components; ++i)
					cDefVal[i] = static_cast<byte>(defaultVal[i] * 255);

				map = Texture::CreateDefaultTexture(m_parent->GetDiskAssetManager(), &cDefVal, 1u, 1u, components, DR_LOW, m_label + "-default-" + type);
				break;

			case DR_HIGH:
				map = Texture::CreateDefaultTexture(m_parent->GetDiskAssetManager(), &defaultVal, 1u, 1u, components, DR_HIGH, m_label + "-default-" + type);
				break;
			}
		}

		return map;
	}

	bool XMaterial::Load(const tinyxml2::XMLElement* xmlData, const std::string& mapsPathRootHint)
	{
		ReadStringAttribute(xmlData, "name", m_label);

		RT_XENGINE_LOG_INFO("Loading XMaterial, name: {}", m_label);
	
		// SetIdentificationFromAssociatedDiskAssetIdentification base maps
		for (auto* materialDataElement = xmlData->FirstChildElement(); materialDataElement != nullptr;
			materialDataElement = materialDataElement->NextSiblingElement())
		{
			const std::string type = materialDataElement->Name();

			const auto* attrPath = materialDataElement->GetText();

			std::string path{};
			if(attrPath)
				path = mapsPathRootHint + "\\" + std::string(attrPath);

			if (Core::CaseInsensitiveCompare(type, "Albedo"))
			{
				m_mapAlbedo = LoadMap<glm::vec3>(path, type, materialDataElement, DR_LOW, true);
			}
			else if (Core::CaseInsensitiveCompare(type, "Opacity"))
			{
				m_mapOpacity = LoadMap<glm::vec1>(path, type, materialDataElement, DR_LOW, true);
			}
			else if (Core::CaseInsensitiveCompare(type, "Emission"))
			{
				m_mapEmission = LoadMap<glm::vec3>(path, type, materialDataElement, DR_LOW, true);
			}
			else if (Core::CaseInsensitiveCompare(type, "AmbientOcclusion"))
			{
				m_mapAmbientOcclusion = LoadMap<glm::vec1>(path, type, materialDataElement, DR_LOW, true);
			}
			else if (Core::CaseInsensitiveCompare(type, "Reflectivity"))
			{
				m_mapReflectivity = LoadMap<glm::vec1>(path, type, materialDataElement, DR_LOW, true);
			}
			else if (Core::CaseInsensitiveCompare(type, "Roughness"))
			{
				m_mapRoughness = LoadMap<glm::vec1>(path, type, materialDataElement, DR_LOW, true);
			}
			else if (Core::CaseInsensitiveCompare(type, "Metallic"))
			{
				m_mapMetallic = LoadMap<glm::vec1>(path, type, materialDataElement, DR_LOW, true);
			}
			else if (Core::CaseInsensitiveCompare(type, "Translucency"))
			{
				m_mapTranslucency = LoadMap<glm::vec1>(path, type, materialDataElement, DR_LOW, true);
			}
			else if (Core::CaseInsensitiveCompare(type, "Normal"))
			{
				m_mapNormal = LoadMap<glm::vec3>(path, type, materialDataElement, DR_LOW, false);
			}
			else if (Core::CaseInsensitiveCompare(type, "Height"))
			{
				m_mapHeight = LoadMap<glm::vec1>(path, type, materialDataElement, DR_LOW, true);
			}

			else
				RT_XENGINE_LOG_WARN("Unrecognized XMaterial attribute: {}", type);
		}

		// SetIdentificationFromAssociatedDiskAssetIdentification packed maps

		// RGB: Albedo A: Opacity
		m_mapSurfaceAlbedo = GetDiskAssetManager()->LoadPackedTexture(m_mapAlbedo.get(), 3u,
			nullptr, 0u,
			nullptr, 0u,
			m_mapOpacity.get(), 1u, DR_LOW);

		m_mapSurfaceAlbedo->SetIdentificationFromPath("SurfaceAlbedo");

		// RGB: Emission A: Ambient Occlusion
		m_mapSurfaceEmission = GetDiskAssetManager()->LoadPackedTexture(m_mapEmission.get(), 3u,
			nullptr, 0u,
			nullptr, 0u,
			m_mapAmbientOcclusion.get(), 1u, DR_LOW);

		m_mapSurfaceEmission->SetIdentificationFromPath("SurfaceEmission");

		// R: Reflectivity G: Roughness B: Metallic A: Translucency
		m_mapSurfaceSpecularParameters = GetDiskAssetManager()->LoadPackedTexture(m_mapReflectivity.get(), 1u,
			m_mapRoughness.get(), 1u,
			m_mapMetallic.get(), 1u,
			m_mapTranslucency.get(), 1u,
			DR_LOW);

		m_mapSurfaceSpecularParameters->SetIdentificationFromPath("SurfaceSpecularParameters");

		// RGB: Normal A: Height
		m_mapSurfaceBump = GetDiskAssetManager()->LoadPackedTexture(m_mapNormal.get(), 3u,
			nullptr, 0u,
			nullptr, 0u,
			m_mapHeight.get(), 1u,
			DR_LOW);

		m_mapSurfaceBump->SetIdentificationFromPath("SurfaceBump");

		MarkLoaded();
		
		return true;
	}

	void XMaterial::Clear()
	{
		m_mapAlbedo->Unload();
		m_mapEmission->Unload();
		m_mapReflectivity->Unload();
		m_mapRoughness->Unload();
		m_mapMetallic->Unload();
		m_mapOpacity->Unload();
		m_mapTranslucency->Unload();
		m_mapHeight->Unload();
		m_mapAmbientOcclusion->Unload();

		if (m_mapSurfaceAlbedo) m_mapSurfaceAlbedo->Unload();
		if (m_mapSurfaceEmission) m_mapSurfaceEmission->Unload();
		if (m_mapSurfaceSpecularParameters) m_mapSurfaceSpecularParameters->Unload();
		if (m_mapSurfaceBump) m_mapSurfaceBump->Unload();
	}
}
