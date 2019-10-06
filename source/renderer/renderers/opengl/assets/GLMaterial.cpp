#include "pch.h"

#include "renderer/renderers/opengl/assets/GLMaterial.h"
#include "asset/AssetManager.h"
#include "renderer/renderers/opengl/GLAssetManager.h"

namespace OpenGL
{
	bool GLMaterial::Load()
	{
		auto am = Engine::GetAssetManager();
		
		m_materialPod = AssetManager::GetOrCreate<MaterialPod>(m_assetManagerPodPath);
		
		auto glAm = GetGLAssetManager(this);

		baseColorTexture = glAm->GetOrMakeFromPodHandle<GLTexture>(m_materialPod->baseColorTexture);
		metallicRoughnessTexture = glAm->GetOrMakeFromPodHandle<GLTexture>(m_materialPod->metallicRoughnessTexture);
		occlusionTexture = glAm->GetOrMakeFromPodHandle<GLTexture>(m_materialPod->occlusionTexture);
		normalTexture = glAm->GetOrMakeFromPodHandle<GLTexture>(m_materialPod->normalTexture);
		emissiveTexture = glAm->GetOrMakeFromPodHandle<GLTexture>(m_materialPod->emissiveTexture);
		
		return true;
	}

}
