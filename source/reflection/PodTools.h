#pragma once

#include "reflection/PodReflection.h"
#include "reflection/GetClass.h"

namespace podtools {
namespace detail {
	// Visits pod on operator() of v
	template<typename Visitor, typename... PodTs>
	void PodVisit_Impl(AssetPod* pod, Visitor& v)
	{
		[[maybe_unused]] bool cc = (VisitIf<PodTs>(pod, v) || ...);
	}

	template<typename Visitor, typename... PodTs>
	void PodVisitType_Impl(TypeId hash, Visitor& v)
	{
		[[maybe_unused]] bool cc = (VisitIfType<PodTs>(hash, v) || ...);
	}

	// Runs v(PodCast<T>(pod)) and returns true, when pod is of type T
	template<typename T, typename Visitor>
	bool VisitIf(AssetPod* pod, Visitor& v)
	{
		if (pod->IsOfType<T>()) {
			v(PodCast<T>(pod));
			return true;
		}
		return false;
	}

	// Runs v<T>(T* invalidObject) and returns true, when hash is equal to PodHash<T>
	// T invalidObject is a typed pointer that points to null. Usefull for lambda overload with auto
	template<typename T, typename Visitor>
	bool VisitIfType(TypeId type, Visitor& v)
	{
		if (refl::GetId<T>() == type) {
			v.template operator()<T>();
			return true;
		}
		return false;
	}


	template<typename Visitor, typename... PodTs>
	void PodVisitHash_Impl(mti::Hash hash, Visitor& v)
	{
		[[maybe_unused]] bool cc = (VisitIfHash<PodTs>(hash, v) || ...);
	}


	template<typename T, typename Visitor>
	bool VisitIfHash(mti::Hash hash, Visitor& v)
	{
		if (mti::GetHash<T>() == hash) {
			v.template operator()<T>();
			return true;
		}
		return false;
	}
} // namespace detail


// Visits operator() of Visitor v with the proper pod type after casting it.
template<typename Visitor>
void VisitPod(AssetPod* pod, Visitor& v)
{
	detail::PodVisit_Impl<Visitor, ENGINE_POD_TYPES>(pod, v);
}


// Visits operator() of Visitor v with the proper type
template<typename Visitor>
void VisitPodType(TypeId type, Visitor& v)
{
	detail::PodVisitType_Impl<Visitor, ENGINE_POD_TYPES>(type, v);
}

// Visits operator() of Visitor v with the proper type
template<typename Visitor>
void VisitPodHash(mti::Hash hash, Visitor& v)
{
	detail::PodVisitHash_Impl<Visitor, ENGINE_POD_TYPES>(hash, v);
}


namespace detail {
	template<typename Visitor, typename... PodTs>
	void VisitPerType_Impl(Visitor& visitor)
	{
		(visitor.template operator()<PodTs>(), ...);
	}
} // namespace detail

// Visits operator() of visitor once with each pod type
template<typename Visitor>
void ForEachPodType(Visitor& visitor)
{
	detail::VisitPerType_Impl<Visitor, ENGINE_POD_TYPES>(visitor);
}


namespace detail {
	template<typename MappedType, typename Visitor, typename... PodTs>
	std::unordered_map<TypeId, MappedType> CreateMapOnPodType_Impl(Visitor& visitor)
	{
		std::unordered_map<TypeId, MappedType> result;


		// What this code does: DOC:
		(                                           // Begin expansion of variadic template
													// For each type 'T' in PodTs do this:
			result.insert({                         //	| Insert in result map:
				refl::GetId<PodTs>(),               //  |   At position of type T's hash
				visitor.template operator()<PodTs>( //  |
					) }) //  |   Whatever visitor<T>(T* dummy) returns (see use of DummyType<T>())
			,            // End Expansion
			...);

		return result;
	}
} // namespace detail


// Visitor callabe gets visited on operator() with a single param a dummy Pointer of PodType
// and is expected to return a variable of type MappedType. Visitors return value will get moved into the map.
template<typename MappedType, typename Visitor>
[[nodiscard]] std::unordered_map<TypeId, MappedType> CreateMapOnPodType(Visitor& v)
{
	return detail::CreateMapOnPodType_Impl<MappedType, Visitor, ENGINE_POD_TYPES>(v);
}
} // namespace podtools
