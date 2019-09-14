#include "pch.h"

#include "renderer/renderers/opengl/test/GLTestRenderer.h"
#include "renderer/renderers/opengl/assets/GLMaterial.h"
#include "asset/AssetManager.h"
#include "renderer/renderers/opengl/GLAssetManager.h"

namespace OpenGL
{
	MaterialPod* GLMaterial::GetMaterialAsset() const
	{
		// TODO:
		return m_pod;//  Engine::GetAssetManager()->RequestFreshPod<MaterialPod>(m_assetManagerPodPath);
	}

	bool GLMaterial::Load()
	{
		auto am = Engine::GetAssetManager();
		
		const auto materialData = am->RequestFreshPod<MaterialPod>(m_assetManagerPodPath);
		m_pod = materialData;
		am->RefreshPod(materialData->baseColorTexture);
		am->RefreshPod(materialData->occlusionMetallicRoughnessTexture);
		am->RefreshPod(materialData->normalTexture);
		am->RefreshPod(materialData->emissiveTexture);

		auto glAm = GetGLAssetManager(this);
			
		m_baseColorTexture = glAm->GetOrMakeFromUri<GLTexture>(am->GetPodPath(materialData->baseColorTexture));
		m_occlusionMetallicRoughnessTexture = glAm->GetOrMakeFromUri<GLTexture>(am->GetPodPath(materialData->occlusionMetallicRoughnessTexture));
		m_normalTexture = glAm->GetOrMakeFromUri<GLTexture>(am->GetPodPath(materialData->normalTexture));
		m_emissiveTexture = glAm->GetOrMakeFromUri<GLTexture>(am->GetPodPath(materialData->emissiveTexture));
		
		return true;
	}

}
