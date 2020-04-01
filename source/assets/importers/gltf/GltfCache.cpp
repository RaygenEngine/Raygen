#include "pch.h"
#include "GltfCache.h"

#include "assets/AssetImporterManager.h"

namespace gltfutl {
GltfCache::GltfCache(const fs::path& path)
	: gltfFilePath(path.generic_string())
	, systemPath(path)
{
	tg::TinyGLTF loader;

	std::string err;
	std::string warn;

	filename = path.filename().replace_extension().generic_string();

	const bool ret = loader.LoadASCIIFromFile(&gltfData, &err, &warn, gltfFilePath.c_str());

	CLOG_WARN(!warn.empty(), "Gltf Load warning for {}: {}", path, warn.c_str());
	CLOG_ABORT(!err.empty(), "Gltf Load error for {}: {}", path, err.c_str());

	LoadImages();
	LoadSamplers();
	LoadMaterials();
}


void GltfCache::LoadImages()
{
	// CHECK: embedded images not supported currently
	for (auto& img : gltfData.images) {
		fs::path imgPath = systemPath.remove_filename() / img.uri;
		imagePods.push_back(ImporterManager->ImportRequest<Image>(imgPath));
	}
}

void GltfCache::LoadSamplers()
{
	int32 samplerIndex = 0;
	for (auto& sampler : gltfData.samplers) {

		nlohmann::json data;
		data["sampler"] = samplerIndex;
		auto samplerPath = uri::MakeChildJson(gltfFilePath, data);

		std::string name = sampler.name.empty() ? filename + "_Sampler_" + std::to_string(samplerIndex) : sampler.name;

		auto& [handle, pod] = ImporterManager->CreateEntry<Sampler>(samplerPath, name);

		pod->minFilter = GetTextureFiltering(sampler.minFilter);
		pod->magFilter = GetTextureFiltering(sampler.magFilter);

		// Based on opengl convections min filter contains the mip map filtering information
		pod->mipmapFilter = GetMipmapFiltering(sampler.minFilter);

		pod->wrapU = GetTextureWrapping(sampler.wrapS);
		pod->wrapV = GetTextureWrapping(sampler.wrapT);
		pod->wrapW = GetTextureWrapping(sampler.wrapR);

		samplerPods.push_back(handle);
		samplerIndex++;
	}
}

void GltfCache::LoadMaterial(Material* pod, size_t index)
{
	auto& data = gltfData.materials.at(index);

	// factors
	auto bFactor = data.pbrMetallicRoughness.baseColorFactor;
	pod->baseColorFactor = { bFactor[0], bFactor[1], bFactor[2], bFactor[3] };
	pod->metallicFactor = static_cast<float>(data.pbrMetallicRoughness.metallicFactor);
	pod->roughnessFactor = static_cast<float>(data.pbrMetallicRoughness.roughnessFactor);
	auto eFactor = data.emissiveFactor;
	pod->emissiveFactor = { eFactor[0], eFactor[1], eFactor[2] };

	// scales/strenghts
	pod->normalScale = static_cast<float>(data.normalTexture.scale);
	pod->occlusionStrength = static_cast<float>(data.occlusionTexture.strength);

	// alpha
	pod->alphaMode = GetAlphaMode(data.alphaMode);

	pod->alphaCutoff = static_cast<float>(data.alphaCutoff);
	// doublesided-ness
	pod->doubleSided = data.doubleSided;

	auto fillMatTexture = [&](auto textureInfo, PodHandle<Sampler>& sampler, PodHandle<Image>& image) {
		if (textureInfo.index != -1) {

			auto texture = gltfData.textures.at(textureInfo.index);

			const auto imageIndex = texture.source;
			CLOG_ABORT(imageIndex == -1, "This model is unsafe to use");

			image = imagePods.at(imageIndex);

			const auto samplerIndex = texture.sampler;
			if (samplerIndex != -1) {
				sampler = samplerPods.at(samplerIndex);
			} // else default
		}
	};

	// samplers
	auto& baseColorTextureInfo = data.pbrMetallicRoughness.baseColorTexture;
	fillMatTexture(baseColorTextureInfo, pod->baseColorSampler, pod->baseColorImage);
	auto im = pod->baseColorImage.Lock();
	const_cast<Image*>(im)->format = ImageFormat::Srgb;

	auto& emissiveTextureInfo = data.emissiveTexture;
	fillMatTexture(emissiveTextureInfo, pod->emissiveSampler, pod->emissiveImage);
	im = pod->emissiveImage.Lock();
	const_cast<Image*>(im)->format = ImageFormat::Srgb;

	auto& normalTextureInfo = data.normalTexture;
	fillMatTexture(normalTextureInfo, pod->normalSampler, pod->normalImage);

	auto& metallicRougnessTextureInfo = data.pbrMetallicRoughness.metallicRoughnessTexture;
	fillMatTexture(metallicRougnessTextureInfo, pod->metallicRoughnessSampler, pod->metallicRoughnessImage);

	auto& occlusionTextureInfo = data.occlusionTexture;
	fillMatTexture(occlusionTextureInfo, pod->occlusionSampler, pod->occlusionImage);
}

void GltfCache::LoadMaterials()
{
	int32 matIndex = 0;
	for (auto& mat : gltfData.materials) {

		nlohmann::json data;
		data["material"] = matIndex;
		auto matPath = uri::MakeChildJson(gltfFilePath, data);

		std::string name = mat.name.empty() ? filename + "_Mat_" + std::to_string(matIndex) : mat.name;

		auto& [handle, pod] = ImporterManager->CreateEntry<Material>(matPath, name);

		LoadMaterial(pod, matIndex);

		materialPods.emplace_back(handle);
		matIndex++;
	}

	materialPods.push_back({}); // bleh
}
} // namespace gltfutl
