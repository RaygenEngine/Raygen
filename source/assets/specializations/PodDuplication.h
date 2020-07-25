#pragma once
#include "assets/AssetPod.h"


namespace podspec {

// Wrapper for the Reflected Data Duplication & DuplicateData.
// This is what one should call externally to properly and fully duplicate src into dst.
void Duplicate(AssetPod* src, AssetPod* dst, PodEntry* srcEntry, PodEntry* dstEntry);


} // namespace podspec
