#include "pch.h"

#include "asset/AssetManager.h"
#include "core/reflection/PodTools.h"

size_t AssetManager::NextHandle = 1;

void AssetManager::DeletePod_Impl(AssetPod* pod)
{
	podtools::VisitPod(pod,
		[](auto* pod)
		{
			static_assert(!std::is_same_v<decltype(pod), AssetPod*>, "This should not ever instantiate with AssetPod*. Pod tools has internal error.");
			delete pod;
		}
	);
}

AssetPod* AssetManager::CreatePodByType(TypeId type)
{
	AssetPod* r;
	podtools::VisitPodType(type, [&r](auto typeCarrier) {
		using PodType = std::remove_pointer_t<decltype(typeCarrier)>;
		r = new PodType();
	});
	return r;
}

AssetPod* AssetManager::_DebugUid(size_t a)
{
	return Engine::GetAssetManager()->m_uidToPod.at(a);
}

bool AssetManager::Init(const std::string& applicationPath, const std::string& dataDirectoryName)
{
	return m_pathSystem.Init(applicationPath, dataDirectoryName);
}
