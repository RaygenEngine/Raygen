#pragma once
#include "assets/importers/gltf/GltfUtl.h"

namespace gltfutl {

struct GltfCache;

class GltfSceneToStaticMeshLoader {

	GltfCache& m_cache;
	BasePodHandle m_loadedPod{};

	bool m_tempModelRequiresDefaultMat{ false };

	void LoadGeometryGroup(GeometryGroup& geom, const tg::Primitive& primitiveData, const glm::mat4& transformMat);
	void AppendGeometryGroupToSlot(std::vector<GeometrySlot>& slots, GeometryGroup& group);

public:
	GltfSceneToStaticMeshLoader(GltfCache& cache, tg::Scene& scene);

	[[nodiscard]] BasePodHandle GetLoadedPod() { return m_loadedPod; }
};
} // namespace gltfutl
