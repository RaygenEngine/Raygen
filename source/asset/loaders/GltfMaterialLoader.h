#pragma once

#include "asset/AssetManager.h"
#include "asset/pods/GltfFilePod.h"
#include "asset/pods/MaterialPod.h"
#include "asset/util/GltfAux.h"
#include "asset/loaders/DummyLoader.h"

#include <tiny_gltf.h>

namespace GltfMaterialLoader {
inline void Load(MaterialPod* pod, const uri::Uri& path)
{
	using namespace nlohmann;

	const auto pPath = uri::GetDiskPath(path);
	auto pParent = AssetImporterManager::OLD_ResolveOrImport<GltfFilePod>(pPath + "{}");

	auto data = uri::GetJson(path);

	int32 ext = 0;
	data["material"].get_to(ext);

	const tinygltf::Model& model = pParent.Lock()->data;
	auto& gltfMaterial = model.materials.at(ext);

	// factors
	auto bFactor = gltfMaterial.pbrMetallicRoughness.baseColorFactor;
	pod->baseColorFactor = { bFactor[0], bFactor[1], bFactor[2], bFactor[3] };
	pod->metallicFactor = static_cast<float>(gltfMaterial.pbrMetallicRoughness.metallicFactor);
	pod->roughnessFactor = static_cast<float>(gltfMaterial.pbrMetallicRoughness.roughnessFactor);
	auto eFactor = gltfMaterial.emissiveFactor;
	pod->emissiveFactor = { eFactor[0], eFactor[1], eFactor[2] };

	// scales/strenghts
	pod->normalScale = static_cast<float>(gltfMaterial.normalTexture.scale);
	pod->occlusionStrength = static_cast<float>(gltfMaterial.occlusionTexture.strength);

	// alpha
	pod->alphaMode = gltfaux::GetAlphaMode(gltfMaterial.alphaMode);

	pod->alphaCutoff = static_cast<float>(gltfMaterial.alphaCutoff);
	// doublesided-ness
	pod->doubleSided = gltfMaterial.doubleSided;

	enum class DefType
	{
		White,
		Normal
	};

	auto LoadTexture = [&](auto textureInfo, PodHandle<SamplerPod>& sampler, int32& textCoordIndex, DefType defType,
						   bool isLinear = true) {
		if (textureInfo.index != -1) {
			json data;
			data["texture"] = textureInfo.index;
			data["isLinear"] = isLinear;
			auto textPath = uri::MakeChildJson(path, data);
			sampler = AssetImporterManager::OLD_ResolveOrImport<SamplerPod>(textPath);

			textCoordIndex = textureInfo.texCoord;
		}
		else {
			switch (defType) {
				case DefType::White: sampler = CustomLoader::GetWhiteTexture(); break;
				case DefType::Normal: sampler = CustomLoader::GetNormalTexture(); break;
			}
		}

		return true;
	};

	// samplers
	auto& baseColorTextureInfo = gltfMaterial.pbrMetallicRoughness.baseColorTexture;
	LoadTexture(baseColorTextureInfo, pod->baseColorSampler, pod->baseColorTexcoordIndex, DefType::White, false);

	auto& emissiveTextureInfo = gltfMaterial.emissiveTexture;
	LoadTexture(emissiveTextureInfo, pod->emissiveSampler, pod->emissiveTexcoordIndex, DefType::White, false);

	auto& normalTextureInfo = gltfMaterial.normalTexture;
	LoadTexture(normalTextureInfo, pod->normalSampler, pod->normalTexcoordIndex, DefType::Normal);

	auto& metallicRougnessTextureInfo = gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture;
	LoadTexture(metallicRougnessTextureInfo, pod->metallicRoughnessSampler, pod->metallicRoughnessTexcoordIndex,
		DefType::White);

	auto& occlusionTextureInfo = gltfMaterial.occlusionTexture;
	LoadTexture(occlusionTextureInfo, pod->occlusionSampler, pod->occlusionTexcoordIndex, DefType::White);
}
}; // namespace GltfMaterialLoader
