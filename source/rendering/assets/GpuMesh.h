#pragma once

#include "rendering/assets/GpuAssetBase.h"
#include "rendering/core/Buffer.h"

struct AssetUpdateInfo;
struct Mesh;
template<typename PodTypeT>
struct PodHandle;


// NEXT: use combined buffer?
struct GpuGeometryGroup {
	UniquePtr<VertexBuffer> vertexBuffer;
	UniquePtr<IndexBuffer> indexBuffer;
};


struct GpuMesh : public GpuAssetTemplate<Mesh> {
	std::vector<GpuGeometryGroup> geometryGroups;


	GpuMesh(PodHandle<Mesh> podHandle);

	void Update(const AssetUpdateInfo& info) override final;
	void UpdateGeometry(const AssetUpdateInfo& info);
};
