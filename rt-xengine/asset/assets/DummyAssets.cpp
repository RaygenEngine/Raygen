#include "pch.h"

#include "asset/assets/DummyAssets.h"
#include "asset/assets/ImageAsset.h"
#include "asset/AssetManager.h"

DefaultTexture* DefaultTexture::GetDefault()
{
	return Engine::GetAssetManager()->RequestSearchAsset<DefaultTexture>(__default__texture);
}

bool DefaultTexture::Load()
{
	auto image = ImageAsset::GetDefaultWhite();
	m_pod->image = image->GetPod();
	
	return true;
}

DefaultMaterial* DefaultMaterial::GetDefault()
{
	return Engine::GetAssetManager()->RequestSearchAsset<DefaultMaterial>(__default__material);
}

bool DefaultMaterial::Load()
{
	auto image = DefaultTexture::GetDefault();
	m_pod->baseColorTexture = image->GetPod();
	//m_pod->normalTexture = image->GetPod();
	m_pod->emissiveTexture = image->GetPod();
	m_pod->occlusionMetallicRoughnessTexture = image->GetPod();

	return true;
}
