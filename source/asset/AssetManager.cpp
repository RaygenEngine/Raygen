#include "pch.h"

#include "asset/AssetManager.h"
#include "core/reflection/PodTools.h"
#include "asset/loaders/GltfFileLoader.h"

bool AssetManager::Init(const std::string& applicationPath, const std::string& dataDirectoryName)
{
	m_pods.push_back(std::make_unique<PodEntry>());

	return m_pathSystem.Init(applicationPath, dataDirectoryName);
}

void AssetManager::PreloadGltf(const uri::Uri& gltfModelPath)
{
	auto pParent = AssetManager::GetOrCreate<GltfFilePod>(gltfModelPath);
	
	tinygltf::Model& file = pParent->data;

	for (auto& gltfImage : file.images)
	{
		GetOrCreateFromParent<ImagePod>(gltfImage.uri, pParent);
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
