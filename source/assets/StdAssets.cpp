#include "StdAssets.h"

#include "assets/AssetManager.h"
#include "assets/pods/Image.h"
#include "assets/pods/Mesh.h"

#include "assets/pods/MaterialArchetype.h"

template<typename T>
typename StdAssets::StdAsset<T>::ConstHandleAssigner Load(const char* path)
{
	auto handle = AssetManager->ImportAs<T>(fs::path(path), true);
	return StdAssets::StdAsset<T>::ConstHandleAssigner{ handle.uid };
}


template<typename T>
typename StdAssets::StdAsset<T>::ConstHandleAssigner Generate(void (*l)(T&), const char* name = "^gen")
{
	auto& [handle, pod] = AssetImporterManager->CreateTransientEntry<T>(name);
	l(*pod);
	return StdAssets::StdAsset<T>::ConstHandleAssigner{ handle.uid };
}

void StdAssets::LoadAssets()
{
	AssetImporterManager->SetPushPath("gen-data/engine-assets");

	// ImageSkyBack = Load<Image>("engine-data/default-skybox/back.jpg");


	//
	//
	//

	NormalImage = Generate<Image>(
		[](Image& pod) {
			pod.data[0] = 0x80;
			pod.data[1] = 0x80;
			pod.data[2] = 0xFF;
			pod.data[3] = 0xFF;
		},
		"^NormalImage");


	GltfArchetype = Generate<MaterialArchetype>(
		[](MaterialArchetype& arch) { MaterialArchetype::MakeGltfArchetypeInto(&arch); }, "^GltfArchetype");


	QuadMesh = Generate<Mesh>(
		[](Mesh& mesh) {
			GeometrySlot slot;
			slot.indices = { 2, 1, 0, 1, 2, 3 };
			for (int i = 0; i < 2; ++i) {
				for (int j = 0; j < 2; ++j) {
					Vertex v;
					v.position = { float(i), 0.f, -float(j) };
					v.normal = { 0.f, 1.f, 0.f };
					v.tangent = { 1.f, 0.f, 0.f };
					v.uv = { float(i), float(j) };
					slot.vertices.emplace_back(std::move(v));
				}
			}
			mesh.geometrySlots.emplace_back(std::move(slot));
			mesh.materials.emplace_back(PodHandle<MaterialInstance>{});
		},
		"^QuadMesh");

	UnitCube = Load<Mesh>("engine-data/mesh/unitcube.gltf");

	BrdfLut = Load<Image>("engine-data/image/brdfLut.png");
	//

	AssetImporterManager->PopPath();

	// CHECK: Semi-temporary: Force transient to never save these. Another flag and specific handling of Standard Assets
	// would be optimal

	for (auto& entry : AssetRegistry::Z_GetPods()) {
		entry->transient = true;
	}
}
