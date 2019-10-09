#pragma once
#include "core/reflection/PodReflection.h"

namespace refl {
namespace detail {
	template<typename Visitor, typename... PodTs>
	void PodVisitPConst_Impl(const AssetPod* pod, Visitor& v)
	{
		bool cc = (VisitIfPConst<PodTs>(pod, v) || ...);
	}

	template<typename T, typename Visitor>
	bool VisitIfPConst(const AssetPod* pod, Visitor& v)
	{
		if (pod->IsOfType<T>()) {
			v(PodCast<T>(pod));
			return true;
		}
		return false;
	}

	template<typename Visitor>
	void VisitPodConst(const AssetPod* pod, Visitor& v)
	{
		detail::PodVisitPConst_Impl<Visitor, Z_POD_TYPES>(pod, v);
	}

	template<typename T>
	constexpr bool SupportsGetClassF()
	{
		return std::is_base_of_v<Node, T> || std::is_base_of_v<AssetPod, T>;
	}
} // namespace detail

template<typename T>
const ReflClass& GetClass(const T* obj)
{
	static_assert(detail::SupportsGetClassF<T>(), "Cannot get class from this object type.");
	if constexpr (std::is_base_of_v<Node, T>) { // NOLINT
		// Virtual call to get the lowest reflector even if T == Node
		return obj->GetClass();
	}
	else if constexpr (std::is_base_of_v<AssetPod, T>) { // NOLINT
		if constexpr (std::is_same_v<AssetPod, T>) {     // NOLINT
			const ReflClass* ptr;
			detail::VisitPodConst(
				obj, [&ptr](auto pod) { ptr = &std::remove_pointer_t<std::decay_t<decltype(pod)>>::StaticClass(); });
			return *ptr;
		}
		return T::StaticClass();
	}
}
} // namespace refl
