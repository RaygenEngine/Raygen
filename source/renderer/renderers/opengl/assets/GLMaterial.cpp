#include "pch.h"

#include "renderer/renderers/opengl/test/GLTestRenderer.h"
#include "renderer/renderers/opengl/assets/GLMaterial.h"
#include "asset/AssetManager.h"
#include "renderer/renderers/opengl/GLAssetManager.h"

namespace OpenGL
{
	PodHandle<MaterialPod> GLMaterial::GetMaterialAsset() const
	{
		return m_pod;//  Engine::GetAssetManager()->RequestFreshPod<MaterialPod>(m_assetManagerPodPath);
	}

	bool GLMaterial::Load()
	{
		auto am = Engine::GetAssetManager();
		
		m_pod = AssetManager::GetOrCreate<MaterialPod>(m_assetManagerPodPath);
		
		auto glAm = GetGLAssetManager(this);

		m_baseColorTexture = glAm->GetOrMakeFromUri<GLTexture>(am->GetPodPath(m_pod->baseColorTexture));
		m_occlusionMetallicRoughnessTexture = glAm->GetOrMakeFromUri<GLTexture>(am->GetPodPath(m_pod->occlusionMetallicRoughnessTexture));
		m_normalTexture = glAm->GetOrMakeFromUri<GLTexture>(am->GetPodPath(m_pod->normalTexture));
		m_emissiveTexture = glAm->GetOrMakeFromUri<GLTexture>(am->GetPodPath(m_pod->emissiveTexture));
		
		return true;
	}

}
