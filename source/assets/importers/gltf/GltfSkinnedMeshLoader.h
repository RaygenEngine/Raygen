#pragma once
#include "assets/importers/gltf/GltfUtl.h"
#include "assets/pods/SkinnedMesh.h"

namespace gltfutl {
struct GltfCache;

class GltfSkinnedMeshLoader {

	GltfCache& m_cache;
	BasePodHandle m_loadedPod{};

	bool m_tempModelRequiresDefaultMat{ false };

public:
	GltfSkinnedMeshLoader(GltfCache& cache, uint32 skinIndex, tg::Skin& skin);

	[[nodiscard]] BasePodHandle GetLoadedPod() { return m_loadedPod; }
};
} // namespace gltfutl
