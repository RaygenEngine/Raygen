#pragma once

#include "asset/AssetManager.h"
#include "asset/pods/TexturePod.h"
#include "asset/pods/GltfFilePod.h"
#include "asset/util/GltfAux.h"

namespace GltfTextureLoader {
static void Load(PodEntry* entry, TexturePod* pod, const uri::Uri& path)
{
	const auto pPath = uri::GetDiskPath(path);
	auto pParent = AssetImporterManager::ResolveOrImport<GltfFilePod>(pPath + "{}");

	nlohmann::json j = uri::GetJson(path);
	int32 ext = j["texture"].get<int32>();

	const tinygltf::Model& model = pParent.Lock()->data;

	auto& gltfTexture = model.textures.at(ext);

	const auto imageIndex = gltfTexture.source;

	PodHandle<ImagePod> imgAsset;

	// if image exists

	// TODO: check image settings
	// this should exist (missing is handled from within material)
	CLOG_ABORT(imageIndex == -1, "This model is unsafe to use, handle missing image from material: {}", path);

	if (imageIndex != -1) {
		auto& gltfImage = model.images.at(imageIndex);
		imgAsset = AssetImporterManager::ResolveOrImportFromParentUri<ImagePod>(gltfImage.uri, path);
		entry->name = fmt::format("{}_texture", uri::GetFilenameNoExt(gltfImage.uri));
	}

	pod->image = imgAsset;

	const auto samplerIndex = gltfTexture.sampler;
	// if sampler exists
	if (samplerIndex != -1) {
		auto& gltfSampler = model.samplers.at(samplerIndex);

		if (gltfSampler.name.empty()) {
			entry->name = fmt::format("{}_sampler_{}", uri::GetFilenameNoExt(path), samplerIndex);
		}
		else {
			entry->name = gltfSampler.name;
		}

		// NEXT:
		pod->minFilter = gltfaux::GetTextureFiltering(gltfSampler.minFilter);
		pod->magFilter = gltfaux::GetTextureFiltering(gltfSampler.magFilter);
		pod->wrapU = gltfaux::GetTextureWrapping(gltfSampler.wrapS);
		pod->wrapV = gltfaux::GetTextureWrapping(gltfSampler.wrapT);
		pod->wrapW = gltfaux::GetTextureWrapping(gltfSampler.wrapR);
	}
}
}; // namespace GltfTextureLoader
