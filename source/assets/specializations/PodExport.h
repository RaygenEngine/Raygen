#pragma once
#include "assets/AssetPod.h"

namespace podspec {

// Wrapper for export to disk.
void ExportToDisk(PodEntry* entry, const fs::path& path = {});


// Used writing an asset to disk.
// Path here is guaranteed to be a valid absolute path.
template<CONC(CAssetPod) PodType>
void ExportPod(PodType* pod, const fs::path& path)
{
	LOG_ERROR("Attempting to export to disk a pod of type: {} which has no specialization.", mti::GetName<PodType>());
}

} // namespace podspec
