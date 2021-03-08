#pragma once
#include "assets/PodFwdUtl.h"
#include "core/MetaTemplates.h"


// When adding a pod, add it to engine pod types
// Then add it to PodIncludes.h

#define ENGINE_POD_TYPES                                                                                               \
	Image, Mesh, Shader, Sampler, SkinnedMesh, Animation, Cubemap, ShaderStage, MaterialArchetype, MaterialInstance,   \
		ShaderHeader, Prefab


ENGINE_PODS_FWD(ENGINE_POD_TYPES);


// Returns the default uid for this pod in the asset manager, all pod handles are initialized with the valid
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

constexpr size_t GetPodTypesCount()
{
	return detail::GetMaxDefaultUid<ENGINE_POD_TYPES>();
}


inline constexpr bool BasePodHandle::IsDefault() const
{
	return uid <= GetPodTypesCount();
}


template<typename T>
concept CAssetPod = is_any_of_v<T, ENGINE_POD_TYPES>;
