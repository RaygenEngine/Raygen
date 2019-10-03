#include "pch.h"

#include "asset/AssetManager.h"
#include "core/reflection/PodTools.h"
#include "asset/loaders/GltfFileLoader.h"

bool AssetManager::Init(const std::string& applicationPath, const std::string& dataDirectoryName)
{
	m_pods.push_back({});

	return m_pathSystem.Init(applicationPath, dataDirectoryName);
}

void AssetManager::PreloadGltf(const std::string& gltfPath)
{
	fs::path p = gltfPath;
	auto pParent = AssetManager::GetOrCreate<GltfFilePod>(p.parent_path().filename());

	tinygltf::Model& file = pParent->data;

	for (auto& gltfImage : file.images)
	{
		GetOrCreate<ImagePod>(gltfImage.uri);
	}
}

void PodDeleter::operator()(AssetPod* p)
{
	podtools::VisitPod(p, [](auto* pod)
	{
		static_assert(!std::is_same_v<decltype(pod), AssetPod*>, "This should not ever instantiate with AssetPod*. Pod tools has internal error.");
		delete pod;
	}
	);
}
