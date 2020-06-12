#pragma once

#include "assets/AssetPod.h"
#include "assets/pods/Image.h"
#include "assets/pods/Mesh.h"

namespace podspec {

// Wrapper for the Reflected Data Duplication & DuplicateData.
// This is what one should call externally to properly and fully duplicate src into dst.
void Duplicate(AssetPod* src, AssetPod* dst);


// Used for duplication of non-reflected data.
template<CONC(CAssetPod) PodType>
void DuplicateData(PodType* src, PodType* dst)
{
}

template<>
inline void DuplicateData(Image* src, Image* dst)
{
	dst->data = src->data;
}

template<>
inline void DuplicateData(Mesh* src, Mesh* dst)
{
	dst->geometrySlots = src->geometrySlots;
}


} // namespace podspec
