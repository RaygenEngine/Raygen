#include "GpuMesh.h"

#include "assets/pods/Mesh.h"
#include "rendering/core/Device.h"
#include "rendering/core/CommandQueue.h"


GpuMesh::GpuMesh(PodHandle<Mesh> podHandle)
	: GpuAssetTemplate(podHandle)
{
	auto data = podHandle.Lock();

	UpdateGeometry({});
	Update({});
}

// PERF: based on asset update info should update only mats, accel struct, etc
void GpuMesh::Update(const AssetUpdateInfo& info)
{
	// auto data = podHandle.Lock();

	// for (int32 i = 0; auto& gg : geometryGroups) {
	//	// gg.material = GpuAssetManager->GetGpuHandle(data->materials[i]);
	//	++i;

	//	if (i > data->materials.size()) {
	//		LOG_WARN(
	//			"Incompatible size of GpuMesh, Gpu Geom Groups did not match Cpu groups. Fix this for runtime meshes.");
	//		return;
	//	}
	//}
}

void GpuMesh::UpdateGeometry(const AssetUpdateInfo& info)
{
	auto data = podHandle.Lock();

	geometryGroups.clear();
	if (data->geometrySlots.empty()) {
		return;
	}

	for (const auto& gg : data->geometrySlots) {
		GpuGeometryGroup vgg;

		vgg.vertexBuffer = std::make_unique<VertexBuffer>(gg.vertices.size(), sizeof(Vertex));
		vgg.indexBuffer = std::make_unique<IndexBuffer>(gg.indices.size(), sizeof(uint32));

		vgg.vertexBuffer->FillData(gg.vertices.data());
		vgg.indexBuffer->FillData(gg.indices.data());

		geometryGroups.emplace_back(std::move(vgg));
	}
}
