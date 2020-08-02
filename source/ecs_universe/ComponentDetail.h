#pragma once

struct SceneCompBase;

namespace componentdetail {
template<typename T, typename = void>
struct HasDirtySubstruct : std::false_type {
};

template<typename T>
struct HasDirtySubstruct<T, std::enable_if_t<std::is_empty_v<typename T::Dirty>>> : std::true_type {
};

template<typename T>
constexpr bool HasDirtySubstructV = HasDirtySubstruct<T>::value;

template<typename T, typename = void>
struct HasCreateDestroySubstructs : std::false_type {
};

template<typename T>
struct HasCreateDestroySubstructs<T,
	std::enable_if_t<std::conjunction_v<std::is_empty<typename T::Create>, std::is_empty<typename T::Destroy>>>>
	: std::true_type {
};

template<typename T>
constexpr bool HasCreateDestorySubstructsV = HasCreateDestroySubstructs<T>::value;

} // namespace componentdetail

template<typename T>
concept CComponent = true;
