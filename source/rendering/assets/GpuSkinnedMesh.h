#pragma once
#include "rendering/assets/GpuAssetBase.h"
#include "rendering/assets/GpuMesh.h"

namespace vl {
struct GpuSkinnedMesh : public GpuAssetTemplate<SkinnedMesh> {
	std::vector<GpuGeometryGroup> geometryGroups;


	GpuSkinnedMesh(PodHandle<SkinnedMesh> podHandle);

	void Update(const AssetUpdateInfo& info) override final;
};
} // namespace vl
