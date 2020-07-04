#pragma once
#include "assets/importers/gltf/GltfUtl.h"

namespace gltfutl {


struct GltfCache {
	uri::Uri gltfFilePath;
	uri::Uri filename;
	fs::path systemPath;

	tg::Model gltfData;

	std::vector<PodHandle<Image>> imagePods;
	std::vector<PodHandle<Sampler>> samplerPods;
	std::vector<PodHandle<MaterialInstance>> materialPods;
	std::vector<PodHandle<Animation>> animationPods;

	GltfCache(const fs::path& path);

private:
	void LoadMaterial(MaterialInstance* inst, size_t index);

	void LoadImages();
	void LoadSamplers();
	void LoadMaterials();
	void LoadAnimations();
};

} // namespace gltfutl
