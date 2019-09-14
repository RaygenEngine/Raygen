#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "asset/pods/MaterialPod.h"

namespace OpenGL
{
	class GLMaterial : public GLAsset
	{
		// RGB: Albedo A: Opacity
		GLTexture* m_baseColorTexture;
		// R: occlusion, G: Roughness, B: Metal, A: empty
		GLTexture* m_occlusionMetallicRoughnessTexture;
		GLTexture* m_normalTexture;
		GLTexture* m_emissiveTexture;
		
	public:
		GLMaterial(const fs::path& assocPath)
			: GLAsset(assocPath),
			  m_baseColorTexture(nullptr),
			  m_occlusionMetallicRoughnessTexture(nullptr),
			  m_normalTexture(nullptr),
			  m_emissiveTexture(nullptr) {}

		[[nodiscard]] GLTexture* GetBaseColorTexture() const { return m_baseColorTexture; }
		[[nodiscard]] GLTexture* GetOcclusionMetallicRoughnessTexture() const { return m_occlusionMetallicRoughnessTexture; }
		[[nodiscard]] GLTexture* GetNormalTexture() const { return m_normalTexture; }
		[[nodiscard]] GLTexture* GetEmissiveTexture() const { return m_emissiveTexture; }

		// PERF:
		MaterialPod* GetMaterialAsset() const;
		
	protected:
		bool Load() override;
		void Unload() override;
	};

}
