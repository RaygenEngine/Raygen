#pragma once
#include "core/MetaTemplates.h"

#include <algorithm>
// When adding a pod, add it to both of these

#define ENGINE_POD_TYPES ::Image, Material, Mesh, Shader, Sampler, SkinnedMesh, Animation, Cubemap

// Can be macroed
struct Image;
struct Material;
struct Mesh;
struct Shader;
struct Sampler;
struct SkinnedMesh;
struct Animation;
struct Cubemap;


// Returns the default uid for this pod in the asset manager, all pod handles are initialized with the the valid
// "default"
template<typename PodType>
constexpr size_t GetDefaultPodUid()
{
	return index_of_type_v<PodType, ENGINE_POD_TYPES> + 1;
}

namespace detail {
template<typename... Args>
constexpr size_t GetMaxDefaultUid()
{
	return std::max({ GetDefaultPodUid<Args>()... });
}
} // namespace detail

constexpr size_t GetDefaultNormalImagePodUid()
{
	return detail::GetMaxDefaultUid<ENGINE_POD_TYPES>() + 1;
}
