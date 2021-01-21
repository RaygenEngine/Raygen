#include "PodDuplication.h"

#include "assets/PodEditor.h"
#include "assets/StdAssets.h"
#include "reflection/PodTools.h"
#include "reflection/ReflectionTools.h"

#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

namespace {

// Used for duplication of non-reflected data.
template<CAssetPod PodType>
void DuplicateData(PodType* src, PodType* dst, PodEntry* srcEntry, PodEntry* dstEntry)
{
}

template<>
inline void DuplicateData(Image* src, Image* dst, PodEntry* srcEntry, PodEntry* dstEntry)
{
	dst->data = src->data;
}

template<>
inline void DuplicateData(Mesh* src, Mesh* dst, PodEntry* srcEntry, PodEntry* dstEntry)
{
	dst->geometrySlots = src->geometrySlots;
}

template<>
inline void DuplicateData(MaterialArchetype* src, MaterialArchetype* dst, PodEntry* srcEntry, PodEntry* dstEntry)
{
	dst->gbufferFragBinary = src->gbufferFragBinary;
	dst->gbufferVertBinary = src->gbufferVertBinary;
	dst->depthFragBinary = src->depthFragBinary;
	dst->depthVertBinary = src->depthVertBinary;
	dst->unlitFragBinary = src->unlitFragBinary;


	// Serialize & Deserialize descriptorSetLayout (instead of bothering with copy) for now.
	// Code is tested and possibly even faster than the default generated copy due to the underlying RuntimeClass.
	std::stringstream stream;
	{
		cereal::BinaryOutputArchive arc(stream);
		arc(src->descriptorSetLayout);
	} // Ensure flash of arc archive (by sending it out of scope.)

	{
		cereal::BinaryInputArchive outarch(stream);
		outarch(dst->descriptorSetLayout);
	}
}

template<>
inline void DuplicateData(MaterialInstance* src, MaterialInstance* dst, PodEntry* srcEntry, PodEntry* dstEntry)
{
	dst->archetype = src->archetype;
	dst->descriptorSet = src->descriptorSet;

	if (dst->archetype != StdAssets::GltfArchetype()) {
		PodEditor ed(dst->archetype);
		ed->instances.push_back(dstEntry->GetHandleAs<MaterialInstance>());
	}
}
} // namespace

namespace podspec {
void Duplicate(AssetPod* src, AssetPod* dst, PodEntry* srcEntry, PodEntry* dstEntry)
{
	auto result = refltools::CopyClassTo(src, dst);
	CLOG_ERROR(!result.IsExactlyCorrect(), "Duplicate pod had errors! Are you sure the pods are the same type?");

	podtools::VisitPod(src, [dst, srcEntry, dstEntry]<typename T>(T* src_cast) { //
		DuplicateData<T>(src_cast, static_cast<T*>(dst), srcEntry, dstEntry);
	});
}
} // namespace podspec
