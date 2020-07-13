#pragma once
#include "assets/importers/gltf/GltfUtl.h"
#include "assets/pods/SkinnedMesh.h"

namespace gltfutl {
struct GltfCache;

class GltfSkinnedMeshLoader {

	GltfCache& cache;

	PodHandle<SkinnedMesh> skinHandle;
	SkinnedMesh* skinPod;
	uint32 skinIndex;
	tg::Skin& gltfSkin;

	void LoadAnimations();

	void LoadSkinMesh();

	void SortJoints();

	int32 NodeToJoint(int32 nodeIndex);

	std::vector<int32> jointRemap;

public:
	GltfSkinnedMeshLoader(GltfCache& inCache, uint32 inSkinIndex, tg::Skin& skin);

	[[nodiscard]] BasePodHandle GetLoadedPod() { return skinHandle; }
};
} // namespace gltfutl
