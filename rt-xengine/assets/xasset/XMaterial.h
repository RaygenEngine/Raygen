#ifndef XMATERIAL_H
#define XMATERIAL_H

#include "assets/DiskAssetPart.h"

#include "assets/texture/Texture.h"

#include "tinyxml2/tinyxml2.h"

namespace Assets
{
	/*
 * SpecularParameters R: refle G: rough B: Metal
   Albedo RGB : Albedo
   Emission RGB: Emission
   Normal RGB: Normal A: height
   SurfaceParameters R: Translu B: AmbientOcc G: Opacity
 */

	class XMaterial : public DiskAssetPart
	{
		template<typename DefaultValueType>
		std::shared_ptr<Texture> LoadMap(const std::string& path, const std::string& type, const tinyxml2::XMLElement* materialData, DYNAMIC_RANGE dr, bool loadDefaultIfMissing);

		// Base textures

		// Albedo
		std::shared_ptr<Texture> m_mapAlbedo;
		// Opacity
		std::shared_ptr<Texture> m_mapOpacity;

		// Emission
		std::shared_ptr<Texture> m_mapEmission;
		// Ambient occlusion
		std::shared_ptr<Texture> m_mapAmbientOcclusion;

		// Reflectivity 
		std::shared_ptr<Texture> m_mapReflectivity;
		// Roughness
		std::shared_ptr<Texture> m_mapRoughness;
		// Metallic
		std::shared_ptr<Texture> m_mapMetallic;
		// Translucency
		std::shared_ptr<Texture> m_mapTranslucency;

		// Normal
		std::shared_ptr<Texture> m_mapNormal;
		// Height
		std::shared_ptr<Texture> m_mapHeight;

		// Packed textures (most used packings, for caching)

		// RGB: Albedo A: Opacity
		std::shared_ptr<PackedTexture> m_mapSurfaceAlbedo;

		// RGB: Emission A: Ambient Occlusion
		std::shared_ptr<PackedTexture> m_mapSurfaceEmission;

		// R: Reflectivity G: Roughness B: Metallic A: Translucency
		std::shared_ptr<PackedTexture> m_mapSurfaceSpecularParameters;

		// RGB: Normal A: Height
		std::shared_ptr<PackedTexture> m_mapSurfaceBump;


	public:
		XMaterial(DiskAsset* parent);
		~XMaterial() = default;

		bool Load(const tinyxml2::XMLElement* xmlData, const std::string& mapsPathRootHint);
		void Clear() override;

		Texture* GetMapAlbedo() const { return m_mapAlbedo.get(); }
		Texture* GetMapEmission() const { return m_mapEmission.get(); }
		Texture* GetMapReflectivity() const { return m_mapReflectivity.get(); }
		Texture* GetMapRoughness() const { return m_mapRoughness.get(); }
		Texture* GetMapMetallic() const { return m_mapMetallic.get(); }
		Texture* GetMapOpacity() const { return m_mapOpacity.get(); }
		Texture* GetMapTranslucency() const { return m_mapTranslucency.get(); }
		Texture* GetMapNormal() const { return m_mapNormal.get(); }
		Texture* GetMapHeight() const { return m_mapHeight.get(); }
		Texture* GetMapAmbientOcclusion() const { return m_mapAmbientOcclusion.get(); }

		PackedTexture* GetMapSurfaceAlbedo() const { return m_mapSurfaceAlbedo.get(); }
		PackedTexture* GetMapSurfaceEmission() const { return m_mapSurfaceEmission.get(); }
		PackedTexture* GetMapSurfaceSpecularParameters() const { return m_mapSurfaceSpecularParameters.get(); }
		PackedTexture* GetMapSurfaceBump() const { return m_mapSurfaceBump.get(); }

		void ToString(std::ostream& os) const override { os << "asset-type: XMaterial, name: " << m_label; }
	};


}

#endif // XMATERIAL_H
