#include "pch.h"

#include "assets/model/Sampler.h"
#include "assets/model/GltfAux.h"

Sampler::Sampler(bool loadDefaultTexture)
	: m_texture(nullptr),
	  m_minFilter(TextureFiltering::LINEAR),
	  m_magFilter(TextureFiltering::LINEAR),
	  m_wrapS(TextureWrapping::REPEAT),
	  m_wrapT(TextureWrapping::REPEAT),
	  m_wrapR(TextureWrapping::REPEAT),
	  m_texCoordIndex(0)
{
	// TODO, make this constexpr?
	if (loadDefaultTexture)
	{
		// LOW DR
		// gltf = if texture missing all values of default will be equal to 1, therefore the factor is the final value
		byte cDefVal[4] = { 255, 255, 255, 255 };
		m_texture = Texture::CreateDefaultTexture(this, &cDefVal, 1u, 1u, 4u, DynamicRange::LOW);
	}
}

void Sampler::Load(const tinygltf::Model& modelData, int32 gltfTextureIndex, int32 gltfTexCoordTarget)
{
	const auto textureIndex = gltfTextureIndex;

	// if texture exists 
	if (textureIndex != -1)
	{
		auto& gltfTexture = modelData.textures.at(textureIndex);

		const auto imageIndex = gltfTexture.source;

		// if image exists
		if (imageIndex != -1)
		{
			// TODO check image settings
			auto& gltfImage = modelData.images.at(imageIndex);

			m_texture = GetDiskAssetManager()->LoadTextureAsset(GetDirectoryPath() + "\\" + gltfImage.uri, DynamicRange::LOW, false);
		}

		const auto samplerIndex = gltfTexture.sampler;

		// if sampler exists
		if (samplerIndex != -1)
		{
			auto& gltfSampler = modelData.samplers.at(samplerIndex);

			m_minFilter = GetTextureFilteringFromGltf(gltfSampler.minFilter);
			m_magFilter = GetTextureFilteringFromGltf(gltfSampler.magFilter);
			m_wrapS = GetTextureWrappingFromGltf(gltfSampler.wrapS);
			m_wrapT = GetTextureWrappingFromGltf(gltfSampler.wrapT);
			m_wrapR = GetTextureWrappingFromGltf(gltfSampler.wrapR);
		}
	}
	
	m_texCoordIndex = gltfTexCoordTarget;
}

