#pragma once
#include "core/MetaTemplates.h"
#include "assets/PodFwdUtl.h"


// When adding a pod, add it to both of these

#define ENGINE_POD_TYPES                                                                                               \
	Image, Mesh, Shader, Sampler, SkinnedMesh, Animation, Cubemap, EnvironmentMap, ShaderStage, MaterialArchetype,     \
		MaterialInstance


ENGINE_PODS_FWD(ENGINE_POD_TYPES);


// Can be macroed
// struct Image;
// struct Mesh;
// struct Shader;
// struct ShaderStage;
// struct Sampler;
// struct SkinnedMesh;
// struct Animation;
// struct Cubemap;
// struct EnvironmentMap;
// struct MaterialArchetype;
// struct MaterialInstance;


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

constexpr size_t GetDefaultGtlfArchetypeUid()
{
	return detail::GetMaxDefaultUid<ENGINE_POD_TYPES>() + 2;
}


constexpr size_t GetPodTypesCount()
{
	return detail::GetMaxDefaultUid<ENGINE_POD_TYPES>();
}


inline constexpr bool BasePodHandle::IsDefault() const
{
	return uid <= GetPodTypesCount();
}


template<typename T>
concept CAssetPod = true; // TODO:
