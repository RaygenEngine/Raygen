#pragma once

#include "asset/PodIncludes.h"
#include "core/reflection/TypeId.h"
#include <type_traits>


// TODO: allow extension of this in "global typelist extensions app header" or something
#define Z_POD_TYPES	\
GltfFilePod,		\
ImagePod,			\
MaterialPod,		\
ModelPod,			\
ShaderPod,			\
StringPod,			\
TexturePod,			\
XMLDocPod


// The way the above macro works is that there are always 2 instances of each function. The first is the 'impl'
// that takes a variadic argument list of all the registered pod types (PodTs) and the second, the actual interface
// calls the 'impl' version using the POD_TYPES definition eg: IsRegisteredPod<T>() { IsRegisteredPod_Impl<T, POD_TYPES>() }


namespace refl
{
	namespace detail
	{
		template<typename T, typename... PodTs>
		constexpr bool IsRegisteredPod_Impl = std::disjunction_v<std::is_same<T, PodTs>...>;

		// Tests whether the pod is contained in the POD_TYPES list. You should not have to use this unless you are modifying the PodReflection library
		template<typename T>
		constexpr bool IsRegisteredPod = IsRegisteredPod_Impl<T, Z_POD_TYPES>;

		// Checks if T is properly derived from assetpod. You should not have to use this unless you are modifying the PodReflection library.
		template<typename T>
		constexpr bool HasAssetPodBase = std::is_base_of_v<AssetPod, T>;
	}

	// Checks if T is a valid & registered pod type.
	template<typename T> // Note: if you want to ever change pod requirements you should add the reqiurements here.
	constexpr bool IsValidPod = detail::HasAssetPodBase<T> && detail::IsRegisteredPod<T>;
}


template<typename Type>
[[nodiscard]]
Type* PodCast(AssetPod* pod)
{
	static_assert(refl::IsValidPod<Type>, "This is not a valid and registered asset pod. The cast would always fail.");

	if (refl::GetId<Type>() == pod->type)
	{
		return static_cast<Type*>(pod);
	}
	return nullptr;
}

// This pod cast will assert when fails.
// This is extremelly usefull to catch early errors and avoid incorrect handles.
template<typename Type>
[[nodiscard]]
Type* PodCastVerfied(AssetPod* pod)
{
	static_assert(refl::IsValidPod<Type>, "This is not a valid and registered asset pod. The cast would always fail.");

	if (refl::GetId<Type>() == pod->type)
	{
		return static_cast<Type*>(pod);
	}
	LOG_ASSERT("Verified Pod Cast failed. Tried to cast from: {} to {}", pod->type.name(), refl::GetName<Type>());
	return nullptr;
}


inline const ReflClass& GetPodClass(AssetPod* pod)
{
	// TODO: implement
	assert(false && "Implement me");
	return ReflClass();
}

