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
		// R: *occlusion, G: Roughness, B: Metal, A: empty
		GLTexture* m_metallicRoughnessTexture;
		// R: occlusion, accessing other channels may give wrong info
		GLTexture* m_occlusionTexture;
		GLTexture* m_normalTexture;
		GLTexture* m_emissiveTexture;
		
		PodHandle<MaterialPod> m_pod;
	public:
		GLMaterial(const fs::path& assocPath)
			: GLAsset(assocPath),
			  m_baseColorTexture(nullptr),
			  m_metallicRoughnessTexture(nullptr),
		      m_occlusionTexture(nullptr),
			  m_normalTexture(nullptr),
			  m_emissiveTexture(nullptr)
		{
		}

		friend class GLAssetManager;
	public:
		[[nodiscard]] GLTexture* GetBaseColorTexture() const { return m_baseColorTexture; }
		[[nodiscard]] GLTexture* GetMetallicRoughnessTexture() const { return m_metallicRoughnessTexture; }
		[[nodiscard]] GLTexture* GetOcclusionTexture() const { return m_occlusionTexture; }
		[[nodiscard]] GLTexture* GetNormalTexture() const { return m_normalTexture; }
		[[nodiscard]] GLTexture* GetEmissiveTexture() const { return m_emissiveTexture; }

		PodHandle<MaterialPod> GetMaterialAsset() const;
		
	protected:
		bool Load() override;
	};

}
