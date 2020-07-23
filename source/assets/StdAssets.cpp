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

	ImageSkyBack = Load<Image>("engine-data/default-skybox/back.jpg");


	//
	//
	//

	ImageWhite = Generate<Image>(
		[](Image& img) {
			img.data[0] = 0xFF;
			img.data[1] = 0xFF;
			img.data[2] = 0xFF;
			img.data[3] = 0xFF;
		},
		"^ImageWhite");


	GltfArchetype = Generate<MaterialArchetype>(
		[](MaterialArchetype& arch) { MaterialArchetype::MakeGltfArchetypeInto(&arch); }, "^GltfArchetype");


	//

	AssetImporterManager->PopPath();

	// CHECK: Semi-temporary: Force transient to never save these. Another flag and specific handling of Standard Assets
	// would be optimal

	for (auto& entry : AssetHandlerManager::Z_GetPods()) {
		entry->transient = true;
	}
}
