#include "pch.h"
#include "GltfCache.h"

#include "assets/AssetImporterManager.h"
#include "assets/importers/gltf/GltfUtl.h"
#include "assets/pods/Image.h"
#include "assets/pods/MaterialArchetype.h"
#include "assets/pods/MaterialInstance.h"
#include "assets/pods/Sampler.h"

#include <nlohmann/json.hpp>
#include <tinygltf/tiny_gltf.h>

namespace gltfutl {
GltfCache::GltfCache(const fs::path& path)
	: gltfFilePath(path.generic_string())
	, systemPath(path)
{
	tg::TinyGLTF loader;

	std::string err;
	std::string warn;

	filename = path.filename().replace_extension().generic_string();
	gltfData = std::make_unique<tinygltf::Model>();

	const bool ret = loader.LoadASCIIFromFile(gltfData.get(), &err, &warn, gltfFilePath.c_str());

	CLOG_WARN(!warn.empty(), "Gltf Load warning for {}: {}", path, warn.c_str());
	CLOG_ABORT(!err.empty(), "Gltf Load error for {}: {}", path, err.c_str());


	AssetImporterManager->PushPath(std::string_view(filename));
	LoadImages();
	// LoadSamplers(); // CHECK: Maybe do some check to report irregular samplers in the future
	LoadMaterials();
	AssetImporterManager->PopPath();
}


void GltfCache::LoadImages()
{
	// CHECK: embedded images not supported currently
	for (auto& img : gltfData->images) {
		fs::path imgPath = systemPath.remove_filename() / img.uri;
		imagePods.push_back(AssetImporterManager->ImportRequest<Image>(imgPath));
	}
}

void GltfCache::LoadSamplers()
{

	int32 samplerIndex = 0;
	for (auto& sampler : gltfData->samplers) {

		nlohmann::json data;
		data["sampler"] = samplerIndex;
		auto samplerPath = uri::MakeChildJson(gltfFilePath, data);

		std::string name = sampler.name.empty() ? filename + "_Sampler_" + std::to_string(samplerIndex) : sampler.name;

		auto& [handle, pod] = AssetImporterManager->CreateEntry<Sampler>(samplerPath, name);

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

void GltfCache::LoadMaterial(MaterialInstance* inst, size_t index)
{
	static PodHandle<MaterialArchetype> gltfArchetypeHandle = MaterialArchetype::GetGltfArchetype();
	auto gltfArchetype = gltfArchetypeHandle.Lock();

	inst->archetype = gltfArchetypeHandle;
	inst->descriptorSet.SwapLayout({}, gltfArchetype->descriptorSetLayout);

	auto setData = [&](std::string_view prop, const auto& value) {
		if (!gltfArchetype->descriptorSetLayout.uboClass.SetPropertyValueByName(
				inst->descriptorSet.uboData, prop, value)) {
			LOG_ERROR("Failed to import Gltf Material Instance Property: {}. Class type mismatch.", prop);
		}
	};

	auto& data = gltfData->materials.at(index);

	// factors
	auto bFactor = data.pbrMetallicRoughness.baseColorFactor;
	setData("baseColorFactor", glm::vec4{ bFactor[0], bFactor[1], bFactor[2], bFactor[3] });
	setData("metallicFactor", static_cast<float>(data.pbrMetallicRoughness.metallicFactor));
	setData("roughnessFactor", static_cast<float>(data.pbrMetallicRoughness.roughnessFactor));

	auto eFactor = data.emissiveFactor;
	setData("emissiveFactor", glm::vec4{ eFactor[0], eFactor[1], eFactor[2], 1.f });

	// scales/strenghts
	setData("normalScale", static_cast<float>(data.normalTexture.scale));
	setData("occlusionStrength", static_cast<float>(data.occlusionTexture.strength));

	// alpha
	setData("mask", int{ GetAlphaMode(data.alphaMode) });
	setData("alphaCutoff", static_cast<float>(data.alphaCutoff));

	// CHECK: double-sidedness This could be a material archetype boolean value in the future, currently unsupported

	// Load Textures into the material
	{
		int32 samplerIndex = 0;

		inst->descriptorSet.samplers2d[3] = { GetDefaultNormalImagePodUid() };

		auto fillNextTexture = [&](auto textureInfo, bool srgb = false) {
			if (textureInfo.index != -1) {
				auto texture = gltfData->textures.at(textureInfo.index);

				const auto imageIndex = texture.source;
				CLOG_ABORT(imageIndex == -1, "This model is unsafe to use");

				inst->descriptorSet.samplers2d[samplerIndex] = imagePods.at(imageIndex);
				if (srgb) {
					auto im = imagePods.at(imageIndex).Lock();
					const_cast<Image*>(im)->format = ImageFormat::Srgb;
				}
			}

			samplerIndex++;
		};

		// samplers
		// NOTE: Code order matters here, each time we fill the next slot. Textures should be loaded in the same order
		// as the samplers in the shader.
		auto& baseColorTextureInfo = data.pbrMetallicRoughness.baseColorTexture;
		fillNextTexture(baseColorTextureInfo, true);

		auto& metallicRougnessTextureInfo = data.pbrMetallicRoughness.metallicRoughnessTexture;
		fillNextTexture(metallicRougnessTextureInfo);

		auto& occlusionTextureInfo = data.occlusionTexture;
		fillNextTexture(occlusionTextureInfo);

		auto& normalTextureInfo = data.normalTexture;
		fillNextTexture(normalTextureInfo);

		auto& emissiveTextureInfo = data.emissiveTexture;
		fillNextTexture(emissiveTextureInfo, true);
	}
}

void GltfCache::LoadMaterials()
{
	int32 matIndex = 0;
	for (auto& mat : gltfData->materials) {

		nlohmann::json data;
		data["material"] = matIndex;
		auto matPath = uri::MakeChildJson(gltfFilePath, data);

		std::string name = mat.name.empty() ? filename + "_Mat_" + std::to_string(matIndex) : mat.name;
		auto& [handleInst, podInst] = AssetImporterManager->CreateEntry<MaterialInstance>(matPath, name);

		LoadMaterial(podInst, matIndex);

		materialPods.emplace_back(handleInst);
		matIndex++;
	}

	materialPods.push_back({});
}


} // namespace gltfutl
