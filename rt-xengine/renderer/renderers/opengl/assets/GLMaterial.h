#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "asset/pods/MaterialPod.h"

namespace OpenGL
{
	class GLMaterial : public GLAsset
	{
		MaterialPod* m_materialData;
		
		// RGB: Albedo A: Opacity
		GLTexture* m_baseColorTexture;
		// R: occlusion, G: Roughness, B: Metal, A: empty
		GLTexture* m_occlusionMetallicRoughnessTexture;
		GLTexture* m_normalTexture;
		GLTexture* m_emissiveTexture;
		
		GLMaterial(MaterialPod* materialData)
			: m_materialData(materialData),
			m_baseColorTexture(nullptr),
			m_occlusionMetallicRoughnessTexture(nullptr),
			m_normalTexture(nullptr),
			m_emissiveTexture(nullptr) {}

		bool Load() override;
		friend class GLAssetManager;
	public:
		[[nodiscard]] GLTexture* GetBaseColorTexture() const { return m_baseColorTexture; }
		[[nodiscard]] GLTexture* GetOcclusionMetallicRoughnessTexture() const { return m_occlusionMetallicRoughnessTexture; }
		[[nodiscard]] GLTexture* GetNormalTexture() const { return m_normalTexture; }
		[[nodiscard]] GLTexture* GetEmissiveTexture() const { return m_emissiveTexture; }

		MaterialPod* GetMaterialAsset() const { return m_materialData; }
	};

}
