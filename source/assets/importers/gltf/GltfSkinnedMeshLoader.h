#pragma once

namespace tinygltf {
struct Skin;
}

namespace gltfutl {
struct GltfCache;

class GltfSkinnedMeshLoader {

	GltfCache& cache;

	PodHandle<SkinnedMesh> skinHandle;
	SkinnedMesh* skinPod;
	uint32 skinIndex;
	tinygltf::Skin& gltfSkin;

	void LoadAnimations();

	void LoadSkinMesh();

	void SortJoints();

	int32 NodeToJoint(int32 nodeIndex);

	std::vector<int32> jointRemap;

public:
	GltfSkinnedMeshLoader(GltfCache& inCache, uint32 inSkinIndex, tinygltf::Skin& skin);

	[[nodiscard]] BasePodHandle GetLoadedPod() { return skinHandle; }
};
} // namespace gltfutl
