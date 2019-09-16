#pragma once

#include "asset/AssetManager.h"
#include "asset/pods/TexturePod.h"
#include "asset/pods/GltfFilePod.h"
#include "asset/util/GltfAux.h"

namespace GltfTextureLoader
{
	static bool Load(TexturePod* pod, const fs::path& path)
	{
		const auto pPath = path.parent_path();
		auto pParent = AssetManager::GetOrCreate<GltfFilePod>(pPath);

		const auto info = path.filename();
		const auto ext = std::stoi(&info.extension().string()[1]);

		tinygltf::Model& model = pParent->data;

		auto& gltfTexture = model.textures.at(ext);

		const auto imageIndex = gltfTexture.source;

		auto imgAsset = AssetManager::GetOrCreate<ImagePod>(__default__imageWhite);

		// if image exists
		if (imageIndex != -1)
		{
			// TODO check image settings
			auto& gltfImage = model.images.at(imageIndex);

			auto textPath = pPath.parent_path() / gltfImage.uri;

			imgAsset = AssetManager::GetOrCreate<ImagePod>(textPath);
		}

		pod->image = imgAsset;

		const auto samplerIndex = gltfTexture.sampler;
		// if sampler exists
		if (samplerIndex != -1)
		{
			auto& gltfSampler = model.samplers.at(samplerIndex);

			pod->minFilter = GltfAux::GetTextureFiltering(gltfSampler.minFilter);
			pod->magFilter = GltfAux::GetTextureFiltering(gltfSampler.magFilter);
			pod->wrapS = GltfAux::GetTextureWrapping(gltfSampler.wrapS);
			pod->wrapT = GltfAux::GetTextureWrapping(gltfSampler.wrapT);
			pod->wrapR = GltfAux::GetTextureWrapping(gltfSampler.wrapR);
		}

		//else keep default values
		return true;
	}
};
