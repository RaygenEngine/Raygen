#include "pch.h"
#include "StdAssets.h"

#include "assets/AssetManager.h"
#include "assets/pods/Image.h"
#include "assets/AssetManager.h"

#include "assets/pods/MaterialArchetype.h"

template<typename T>
typename StdAssets::StdAsset<T>::ConstHandleAssigner Load(const char* path)
{
	auto handle = AssetManager->ImportAs<T>(fs::path(path), true);
	return StdAssets::StdAsset<T>::ConstHandleAssigner{};
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


	//

	AssetImporterManager->PopPath();

	// CHECK: Semi-temporary: Force transient to never save these. Another flag and specific handling of Standard Assets
	// would be optimal

	for (auto& entry : AssetRegistry::Z_GetPods()) {
		entry->transient = true;
	}
}
