#pragma once

#include "asset/AssetManager.h"
#include "asset/pods/TexturePod.h"
#include "asset/pods/GltfFilePod.h"
#include "asset/util/GltfAux.h"

namespace GltfTextureLoader
{
	static bool Load(TexturePod* pod, const uri::Uri& path)
	{
		const auto pPath = uri::GetDiskPath(path);
		auto pParent = AssetManager::GetOrCreate<GltfFilePod>(pPath + "{}");

		nlohmann::json j = uri::GetJson(path);
		int32 ext = j["texture"].get<int32>();

		const tinygltf::Model& model = pParent->data;

		auto& gltfTexture = model.textures.at(ext);

		const auto imageIndex = gltfTexture.source;

		auto imgAsset = AssetManager::GetOrCreate<ImagePod>(__default__imageMissing);

		// if image exists

		// TODO check image settings
		// this should exist (missing is handled from within material)
		if (imageIndex != -1)
		{
			auto& gltfImage = model.images.at(imageIndex);
			imgAsset = AssetManager::GetOrCreateFromParentUri<ImagePod>(gltfImage.uri, path);
		}
		else
		{
			LOG_ANY("");
			assert(false && "This model is unsafe to use, handle missing image from material");
		}

		pod->images.push_back(imgAsset);

		const auto samplerIndex = gltfTexture.sampler;
		// if sampler exists
		if (samplerIndex != -1)
		{
			auto& gltfSampler = model.samplers.at(samplerIndex);

			if (gltfSampler.name.empty())
			{
				AssetManager::SetPodName(path, "Sampler." + std::to_string(samplerIndex));
			}
			else
			{
				AssetManager::SetPodName(path, gltfSampler.name);
			}

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
