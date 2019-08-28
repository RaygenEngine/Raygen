#pragma once

#include "assets/DiskAssetPart.h"
#include "assets/texture/Texture.h"

namespace tinygltf
{
	class Model;
	struct Sampler;
	struct Image;
}

namespace Assets
{
	class Sampler : public DiskAssetPart
	{
		std::shared_ptr<Texture> m_texture;

		TextureFiltering m_minFilter;
		TextureFiltering m_magFilter;

		TextureWrapping m_wrapS;
		TextureWrapping m_wrapT;
		TextureWrapping m_wrapR;

		int32 m_texCoordIndex;

	public:
		
		Sampler(DiskAsset* pAsset, const std::string& name);

		void LoadFromGltfData(const tinygltf::Model& modelData, int32 gltfTextureIndex, int32 gltfTexCoordTarget, bool loadDefaultTexture = true);

		TextureFiltering GetMinFilter() const { return m_minFilter; }
		TextureFiltering GetMagFilter() const { return m_magFilter; }
		TextureWrapping GetWrapS() const { return m_wrapS; }
		TextureWrapping GetWrapT() const { return m_wrapT; }
		TextureWrapping GetWrapR() const { return m_wrapR; }
		int32 GetTexCoordIndex() const { return m_texCoordIndex; }

		Texture* GetTexture() const { return m_texture.get(); }

		void ToString(std::ostream& os) const override { os << "asset-type: Sampler, name: " << m_name; }
	};
}
