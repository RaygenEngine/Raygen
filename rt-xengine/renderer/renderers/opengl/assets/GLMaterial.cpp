#include "pch.h"

#include "renderer/renderers/opengl/assets/GLMaterial.h"
#include "assets/AssetManager.h"

namespace OpenGL
{
	bool GLMaterial::Load()
	{
		if (!Engine::GetAssetManager()->Load(m_materialData))
			return false;

		m_baseColorTexture = GetGLAssetManager(this)->MaybeGenerateAsset<GLTexture>(m_materialData->GetBaseColorTexture());
		if (!GetGLAssetManager(this)->Load(m_baseColorTexture))
			return false;

		m_occlusionMetallicRoughnessTexture = GetGLAssetManager(this)->MaybeGenerateAsset<GLTexture>(m_materialData->GetOcclusionMetallicRoughnessTexture());
		if (!GetGLAssetManager(this)->Load(m_occlusionMetallicRoughnessTexture))
			return false;

		m_normalTexture = GetGLAssetManager(this)->MaybeGenerateAsset<GLTexture>(m_materialData->GetNormalTexture());
		if (!GetGLAssetManager(this)->Load(m_normalTexture))
			return false;

		m_emissiveTexture = GetGLAssetManager(this)->MaybeGenerateAsset<GLTexture>(m_materialData->GetEmissiveTexture());
		if (!GetGLAssetManager(this)->Load(m_emissiveTexture))
			return false;
		
		return true;
	}

	void GLMaterial::Unload()
	{
	}
}
