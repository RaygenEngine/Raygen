#include "GpuAssetBase.h"

#include "rendering/assets/GpuAssetManager.h"

std::vector<size_t> GpuAssetBase::GetUsers()
{
	return GpuAssetManager->GetUsersFor(podUid);
}

void GpuAssetBase::AddDependency(BasePodHandle paramAsset)
{
	dependencies.push_back(paramAsset.uid);
	GpuAssetManager->GetUsersRef(paramAsset.uid).push_back(podUid);
}

void GpuAssetBase::ClearDependencies()
{
	// Structures in the dependency system are optimised for small dependency counts therefore we prefer linear search
	// in a cache line instead of heap hash maps

	// Erase ourselves from each of the dependencies
	for (auto& dep : dependencies) {
		auto& vec = GpuAssetManager->GetUsersRef(dep);
		std::erase_if(vec, [&](auto& elem) { return elem == podUid; });
	}
	dependencies.clear();
}
