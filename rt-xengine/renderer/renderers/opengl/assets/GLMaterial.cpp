#include "pch.h"

#include "renderer/renderers/opengl/assets/GLMaterial.h"
#include "asset/AssetManager.h"
#include "renderer/renderers/opengl/GLAssetManager.h"

namespace OpenGL
{
	MaterialPod* GLMaterial::GetMaterialAsset() const
	{
		return Engine::GetAssetManager()->RequestFreshPod<MaterialPod>(m_assetManagerPodPath);
	}

	bool GLMaterial::Load()
	{
		auto am = Engine::GetAssetManager();
		
		const auto materialData = am->RequestFreshPod<MaterialPod>(m_assetManagerPodPath);
		
		am->RefreshPod(materialData->baseColorTexture);
		//am->RefreshPod(materialData->occlusionMetallicRoughnessTexture);
		//am->RefreshPod(materialData->normalTexture);
		//am->RefreshPod(materialData->emissiveTexture);

		auto glAm = GetGLAssetManager(this);
		
		m_baseColorTexture = glAm->RequestLoadAsset<GLTexture>(am->GetPodPath(materialData->baseColorTexture));
		//m_occlusionMetallicRoughnessTexture = glAm->RequestLoadAsset<GLTexture>(am->GetPodPath(materialData->occlusionMetallicRoughnessTexture));
		//m_normalTexture = glAm->RequestLoadAsset<GLTexture>(am->GetPodPath(materialData->normalTexture));
		//m_emissiveTexture = glAm->RequestLoadAsset<GLTexture>(am->GetPodPath(materialData->emissiveTexture));
		
		return true;
	}

	void GLMaterial::Unload()
	{
	}
}
