#pragma once
#include "assets/PodHandle.h"

namespace tinygltf {
struct Scene;
}

namespace gltfutl {
struct GltfCache;

class GltfSceneToStaticMeshLoader {

	GltfCache& m_cache;
	BasePodHandle m_loadedPod{};

	bool m_tempModelRequiresDefaultMat{ false };

public:
	GltfSceneToStaticMeshLoader(GltfCache& cache, tinygltf::Scene& scene);

	[[nodiscard]] BasePodHandle GetLoadedPod() { return m_loadedPod; }
};
} // namespace gltfutl
