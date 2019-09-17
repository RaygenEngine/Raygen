#include "pch.h"

#include "renderer/renderers/opengl/test/GLTestRenderer.h"
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

		baseColorTexture = glAm->GetOrMakeFromUri<GLTexture>(am->GetPodPath(m_materialPod->baseColorTexture));
		metallicRoughnessTexture = glAm->GetOrMakeFromUri<GLTexture>(am->GetPodPath(m_materialPod->metallicRoughnessTexture));
		occlusionTexture = glAm->GetOrMakeFromUri<GLTexture>(am->GetPodPath(m_materialPod->occlusionTexture));
		normalTexture = glAm->GetOrMakeFromUri<GLTexture>(am->GetPodPath(m_materialPod->normalTexture));
		emissiveTexture = glAm->GetOrMakeFromUri<GLTexture>(am->GetPodPath(m_materialPod->emissiveTexture));
		
		return true;
	}

}
