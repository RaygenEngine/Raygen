#pragma once
#include "assets/PodHandle.h"
#include "assets/UriLibrary.h"


namespace tinygltf {
class Model;
}

namespace gltfutl {
struct GltfCache {
	uri::Uri gltfFilePath;
	uri::Uri filename;
	fs::path systemPath;

	UniquePtr<tinygltf::Model> gltfData;

	std::vector<PodHandle<Image>> imagePods;
	std::vector<PodHandle<Sampler>> samplerPods;

	// Contains the default material at the last slot.
	std::vector<PodHandle<MaterialInstance>> materialPods;

	GltfCache(const fs::path& path);

private:
	void LoadMaterial(MaterialInstance* inst, size_t index);

	void LoadImages();
	void LoadSamplers();
	void LoadMaterials();
};
} // namespace gltfutl
