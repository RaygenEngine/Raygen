#pragma once
#include "assets/importers/gltf/GltfUtl.h"
#include "assets/pods/Mesh.h"

namespace gltfutl {
class GltfSceneToStaticMeshLoader {

	GltfCache& m_cache;
	BasePodHandle m_loadedPod{};

	bool m_tempModelRequiresDefaultMat{ false };

public:
	GltfSceneToStaticMeshLoader(GltfCache& cache, tg::Scene& scene);

	[[nodiscard]] BasePodHandle GetLoadedPod() { return m_loadedPod; }
};
} // namespace gltfutl
