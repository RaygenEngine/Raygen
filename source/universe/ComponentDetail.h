#pragma once

struct SceneCompBase;
class Entity;

namespace componentdetail {
// TODO: Use concepts here
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

	// Has BeginEnd Play
	template<typename T, typename = void>
	struct HasBeginEndPlaySubstructs : std::false_type {
	};

	template<typename T>
	struct HasBeginEndPlaySubstructs<T,
		std::enable_if_t<std::conjunction_v<std::is_empty<typename T::BeginPlay>, std::is_empty<typename T::EndPlay>>>>
		: std::true_type {
	};


	// Has BeginEnd Play
	template<typename T, typename = void>
	struct HasTickableSubstruct : std::false_type {
	};

	template<typename T>
	struct HasTickableSubstruct<T, std::enable_if_t<std::is_empty_v<typename T::Tickable>>> : std::true_type {
	};

	template<typename T>
	concept HasSelfMemberVariableEntity = requires(T inst)
	{
		requires(std::is_same_v<decltype(inst.self), Entity>);
	};

	template<typename T, typename = void>
	struct HasSelfMemberVariableEntity : std::false_type {
	};

	template<typename T>
	struct HasSelfMemberVariableEntity<T, std::enable_if_t<std::is_same_v<decltype(T().self), Entity>>>
		: std::true_type {
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

template<typename T>
constexpr bool HasBeginEndPlaySubstructsV = impl::HasBeginEndPlaySubstructs<T>::value;

template<typename T>
constexpr bool HasTickableSubstructV = impl::HasTickableSubstruct<T>::value;

//
template<typename T>
constexpr bool IsSceneComponent = HasSceneTypeV<T>&& HasDirtySubstructV<T>&& HasCreateDestroySubstructsV<T>;


template<typename T>
constexpr bool IsTickableComponent = HasTickableSubstructV<T>;

template<typename T>
constexpr bool IsBeginEndPlayComponent = HasBeginEndPlaySubstructsV<T>;

template<typename T>
constexpr bool IsScriptlikeComponent = IsTickableComponent<T> || IsBeginEndPlayComponent<T>;

template<typename T>
constexpr bool HasSelfMemberVariableEntity = impl::HasSelfMemberVariableEntity<T>;

} // namespace componentdetail

template<typename T>
concept CComponent = true;

template<typename T>
concept CSceneComp = CComponent<T>&& componentdetail::IsSceneComponent<T>;

template<typename T>
concept CScriptlikeComp = CComponent<T>&& componentdetail::IsScriptlikeComponent<T>;

template<typename T>
concept CTickableComp = CComponent<T>&& componentdetail::IsTickableComponent<T>;
