#pragma once

#include "reflection/PodReflection.h"
#include "reflection/ReflClass.h"

class Node;

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
		detail::PodVisitPConst_Impl<Visitor, ENGINE_POD_TYPES>(pod, v);
	}


	// CHECK: Reflected concept should be declared somewhere concretely
	template<typename T>
	concept Reflected = requires(T t)
	{
		{
			T::StaticClass()
		}
		->std::convertible_to<ReflClass>;
	};


	template<typename T>
	constexpr bool SupportsGetClassF()
	{
		return std::is_base_of_v<Node, T> || std::is_base_of_v<AssetPod, T> || Reflected<T>;
	}
} // namespace detail

template<typename T>
const ReflClass& GetClass(const T* obj)
{
	static_assert(detail::SupportsGetClassF<T>(), "Cannot get class from this object type.");
	if constexpr (std::is_base_of_v<Node, T>) {
		// Virtual call to get the lowest reflector even if T == Node
		return obj->GetClass();
	}
	else if constexpr (std::is_base_of_v<AssetPod, T>) {
		if constexpr (std::is_same_v<AssetPod, T>) {
			const ReflClass* ptr;
			auto l = [&ptr](auto pod) {
				ptr = &std::remove_pointer_t<std::decay_t<decltype(pod)>>::StaticClass();
			};
			detail::VisitPodConst(obj, l);
			return *ptr;
		}
		else {
			return T::StaticClass();
		}
	}
	else {
		return T::StaticClass();
	}
}
} // namespace refl
