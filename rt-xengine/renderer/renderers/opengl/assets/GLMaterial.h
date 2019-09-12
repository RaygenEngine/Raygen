#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "assets/model/MaterialAsset.h"

namespace OpenGL
{
	class GLMaterial : public GLAsset
	{
		MaterialAsset* m_materialData;
		
		// RGB: Albedo A: Opacity
		GLTexture* m_baseColorTexture;
		// R: occlusion, G: Roughness, B: Metal, A: empty
		GLTexture* m_occlusionMetallicRoughnessTexture;
		GLTexture* m_normalTexture;
		GLTexture* m_emissiveTexture;
		
	public:
		GLMaterial(MaterialAsset* materialData)
			: GLAsset(materialData),
			m_materialData(materialData),
			m_baseColorTexture(nullptr),
			m_occlusionMetallicRoughnessTexture(nullptr),
			m_normalTexture(nullptr),
			m_emissiveTexture(nullptr) {}

		[[nodiscard]] GLTexture* GetBaseColorTexture() const { return m_baseColorTexture; }
		[[nodiscard]] GLTexture* GetOcclusionMetallicRoughnessTexture() const { return m_occlusionMetallicRoughnessTexture; }
		[[nodiscard]] GLTexture* GetNormalTexture() const { return m_normalTexture; }
		[[nodiscard]] GLTexture* GetEmissiveTexture() const { return m_emissiveTexture; }

		MaterialAsset* GetMaterialAsset() const { return m_materialData; }
		
	protected:
		bool Load() override;
		void Unload() override;
	};

}
