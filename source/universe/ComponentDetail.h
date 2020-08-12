#pragma once

struct SceneCompBase;

namespace componentdetail {
namespace impl {
	template<typename T, typename = void>
	struct HasDirtySubstruct : std::false_type {
	};

	template<typename T>
	struct HasDirtySubstruct<T, std::enable_if_t<std::is_empty_v<typename T::Dirty>>> : std::true_type {
	};


	template<typename T, typename = void>
	struct HasCreateDestroySubstructs : std::false_type {
	};

	template<typename T>
	struct HasCreateDestroySubstructs<T,
		std::enable_if_t<std::conjunction_v<std::is_empty<typename T::Create>, std::is_empty<typename T::Destroy>>>>
		: std::true_type {
	};


	template<typename T, typename = void>
	struct HasSceneType : std::false_type {
	};

	template<typename T>
	struct HasSceneType<T, std::void_t<typename T::RenderSceneType>> : std::true_type {
	};
} // namespace impl


// Detect if the struct T contains an inner empty struct Dirty
template<typename T>
constexpr bool HasDirtySubstructV = impl::HasDirtySubstruct<T>::value;

// Detect if the struct T contains the inner empty structs Create and Destroy
template<typename T>
constexpr bool HasCreateDestroySubstructsV = impl::HasCreateDestroySubstructs<T>::value;

// Detect if the struct T contains RenderSceneType as type definition. (Detect if the component is declared as a scene
// comp)
template<typename T>
constexpr bool HasSceneTypeV = impl::HasSceneType<T>::value;


//
template<typename T>
constexpr bool IsSceneComponent = HasSceneTypeV<T>&& HasDirtySubstructV<T>&& HasCreateDestroySubstructsV<T>;


} // namespace componentdetail

template<typename T>
concept CComponent = true;

template<typename T>
concept CSceneComp = CComponent<T>&& componentdetail::IsSceneComponent<T>;
