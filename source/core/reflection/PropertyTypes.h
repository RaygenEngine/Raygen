#pragma once

#include "core/auxiliary/MetaTemplates.h"

#define Z_REFL_TYPES int32, bool, float, glm::vec3, glm::vec4, std::string

struct BasePodHandle;

namespace refl {
namespace detail {
	template<typename T>
	constexpr bool IsHandleToPodF()
	{
		if constexpr (std::is_base_of_v<BasePodHandle, T> && !std::is_same_v<BasePodHandle, T>) {
			return true;
		}
		return false;
	}

	template<typename T, typename... ReflTypes>
	constexpr bool CanBePropertyBaseType_ImplF()
	{
		return std::disjunction_v<std::is_same<T, ReflTypes>...>;
	}

	template<typename T>
	constexpr bool CanBePropertyBaseTypeF()
	{
		return CanBePropertyBaseType_ImplF<T, Z_REFL_TYPES>();
	}


	template<typename T>
	constexpr bool CanBePropertyF()
	{
		if constexpr (IsHandleToPodF<T>()) {
			return true;
		}
		else if constexpr (is_vector_of_base_v<T, BasePodHandle>) {
			return true;
		}
		else if constexpr (CanBePropertyBaseTypeF<T>()) {
			return true;
		}
		else if constexpr (std::is_enum_v<T>) {
			return true;
		}
		return false;
	}
} // namespace detail

namespace proptypes {
	template<typename Type>
	constexpr bool IsHandleToPod = detail::IsHandleToPodF<Type>();

	template<typename Type>
	constexpr bool IsVectorOfPodHandles = is_vector_of_base_v<Type, BasePodHandle>;

	template<typename Type>
	constexpr bool IsBaseType = detail::CanBePropertyBaseTypeF<Type>();
} // namespace proptypes

// IsReflected to compile time check if a type can be reflected.
template<typename Type>
constexpr bool CanBeProperty = detail::CanBePropertyF<Type>();
} // namespace refl
