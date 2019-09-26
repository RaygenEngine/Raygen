#pragma once

#include "asset/PodIncludes.h"
#include "ctti/type_id.hpp"
#include "ctti/map.hpp"
#include <type_traits>


// TODO: allow extension of this in "global typelist extensions app header" or something
#define POD_TYPES	\
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


// podd for PodDetail
namespace podd
{
template<typename T, typename... PodTs>
constexpr bool IsRegisteredPod_Impl = std::disjunction_v<std::is_same<T, PodTs>...>;

// Tests whether the pod is contained in the POD_TYPES list. You should not have to use this unless you are modifying the PodReflection library
template<typename T>
constexpr bool IsRegisteredPod = IsRegisteredPod_Impl<T, POD_TYPES>;

// Checks if T is properly derived from assetpod. You should not have to use this unless you are modifying the PodReflection library.
template<typename T>
constexpr bool HasAssetPodBase = std::is_base_of_v<AssetPod, T>;
}

// Checks if T is a valid & registered pod type.
template<typename T> // Note: if you want to ever change pod requirements you should add the reqiurements here.
constexpr bool IsValidPod = podd::HasAssetPodBase<T> && podd::IsRegisteredPod<T>;

// Constexpr returns the hash of this pod type.
template<typename T> 
[[nodiscard]]
constexpr auto GetPodTypeHash() { static_assert(IsValidPod<T>); return ctti::unnamed_type_id<T>().hash(); }

// TODO: move to AssetPod struct
namespace pod
{
// returns true when pod is a pod of type T
template<typename T>
[[nodiscard]]
bool IsOfType(AssetPod* pod)
{
	static_assert(IsValidPod<T>, "T is not a pod type.");
	return pod->type.hash() == GetPodTypeHash<T>();
}
}

template<typename Type>
[[nodiscard]]
Type* PodCast(AssetPod* pod)
{
	static_assert(IsValidPod<Type>, "This is not a valid and registered asset pod. The cast would always fail.");

	if (pod::IsOfType<Type>(pod))
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
	static_assert(IsValidPod<Type>, "This is not a valid and registered asset pod. The cast would always fail.");

	if (pod::IsOfType<Type>(pod))
	{
		return static_cast<Type*>(pod);
	}
	LOG_FATAL("Verified Pod Cast failed. Tried to cast from: {} to {}", pod->type.name(), ctti::nameof<Type>());
	assert(false && "See console.");
	return nullptr;
}


namespace podd
{
// Runs v(PodCast<T>(pod)) and returns true, when pod is of type T
template<typename T, typename Visitor>
bool VisitIf(AssetPod* pod, Visitor& v)
{
	if (pod::IsOfType<T>(pod))
	{
		v(PodCast<T>(pod));
		return true;
	}
	return false;
}

// Visits pod on operator() of v
template<typename Visitor, typename... PodTs>
void PodVisit_Impl(AssetPod* pod, Visitor& v)
{
	bool cc = (VisitIf<PodTs>(pod, v) || ...);
}

// Util get nothing but with a type, useful when attempting to do lambda overload with auto param.
// TODO: Fix when c++20 with templated lambda support.
template<typename T>
T* DummyType()
{
	return reinterpret_cast<T*>(nullptr);
}

// Runs v<T>(T* invalidObject) and returns true, when hash is equal to PodHash<T>
// T invalidObject is a typed pointer that points to null. Usefull for lambda overload with auto
template<typename T, typename Visitor>
bool VisitIfHash(ctti::type_index hash, Visitor& v)
{
	if (GetPodTypeHash<T>() == hash)
	{
		v.operator()<T*>(DummyType<T>());
		return true;
	}
	return false;
}

template<typename Visitor, typename... PodTs>
void PodVisitHash_Impl(ctti::type_index hash, Visitor& v)
{
	bool cc = (VisitIfHash<PodTs>(hash, v) || ...);
}
}

// Visits operator() of Visitor v with the proper pod type after casting it.
template<typename Visitor>
void VisitPod(AssetPod* pod, Visitor& v)
{
	podd::PodVisit_Impl<Visitor, POD_TYPES>(pod, v);
}

template<typename Visitor>
void VisitPodHash(ctti::type_index hash, Visitor& v)
{
	podd::PodVisitHash_Impl<Visitor, POD_TYPES>(hash, v);
}

// Properly deletes a generic pod without rtti.
inline void DeletePod(AssetPod* pod)
{
	VisitPod(pod, [](auto pod) {
		static_assert(!std::is_same_v<decltype(pod), AssetPod*>, "This is incorrect code. The visitor should not fire with type of AssetPod*");
		delete pod;
	});
}

// Creates a new instance of the underlying type of the given pod.
[[nodiscard]] inline AssetPod* CreatePodByHash(ctti::type_index hash)
{
	AssetPod* r;
	VisitPodHash(hash, [&r](auto typeCarrier) {
		using PodType = std::remove_pointer_t<decltype(typeCarrier)>;
		r = new PodType();
	});
	return r;
}

// Creates a new instance of the underlying type of the given pod.
[[nodiscard]] inline AssetPod* CreateSamePod(AssetPod* pod)
{
	return CreatePodByHash(pod->type);
}

namespace podd
{

template<typename Visitor, typename... PodTs>
[[nodiscard]]
void VisitPerType_Impl(Visitor& visitor)
{
	(visitor.operator()<PodTs*> (DummyType<PodTs>()), ...);
}

template<typename Second, typename Visitor, typename... PodTs>
[[nodiscard]]
std::unordered_map<ctti::type_id_t, Second> CreateMapOnPodType_Impl(Visitor& visitor)
{
	std::unordered_map<ctti::type_id_t, Second> result;

	// What this code does:
	(															// Begin expansion of variadic template 
																// For each type 'T' in PodTs do this:
		result.insert({											//	| Insert in result map:
			ctti::type_id<PodTs>(),								//  |   At position of type T's hash
			visitor.operator()<PodTs*>(DummyType<PodTs>()) })	//  |   Whatever visitor<T>(T* dummy) returns (see use of DummyType<T>())
	,															// End Expansion
	...);
	
	return result;
}

}

// Visitor callabe gets visited on operator() with a single param a dummy Pointer of PodType
// and is expected to return a variable of type Second. Visitors return value will get moved into the map.
template<typename SecondT, typename Visitor>
[[nodiscard]] 
std::unordered_map<ctti::type_id_t, SecondT> CreateMapOnPodType(Visitor& v)
{
	return podd::CreateMapOnPodType_Impl< SecondT, Visitor, POD_TYPES>(v);
}

template<typename Visitor>
void ForEachPodType(Visitor& visitor)
{
	podd::VisitPerType_Impl<Visitor, POD_TYPES>(visitor);
}

template<typename AnyType>
[[nodiscard]]
ctti::type_id_t GetTypeIdPtr(AnyType* t)
{
	return ctti::type_id<std::remove_pointer_t<AnyType>>();
}

// Class that generates a type->call map for each pod when instanciated and visits the correct pod type in O(1) when called
// from a generic AssetPod* .
template<typename Visitor>
class PodHashVisitor
{
	using ReturnT = return_type_t<decltype(Visitor::operator())>;
	std::unordered_map<ctti::type_id_t, std::function<ReturnT(AssetPod*)>> m_callmap;

	Visitor* m_visitor;

public:
	PodHashVisitor()
	{
		ForEachPodType(
		[&](auto dumRef)
		{
			using PodType = std::remove_pointer_t<decltype(dumRef)>;

			m_callmap.insert(
			{
					ctti::type_id<PodType>(),
					[&](AssetPod* pod) {
						return m_visitor->operator() < PodType > (PodCast<PodType>(pod));
					}
			});
		});
	}

	// Runs the visitor on a single pod.
	// Complexity O(1)
	ReturnT RunVisitor(Visitor& visitor, AssetPod* podThatVisits)
	{
		m_visitor = &visitor;
		return m_callmap[pod->type](podThatVisits);
	}

	// Runs the visitor on a single pod. Handle overload.
	// Complexity O(1)
	template<typename T>
	ReturnT RunVisitor(Visitor& visitor, PodHandle<T> podThatVisits)
	{
		m_visitor = &visitor;
		return m_callmap[ctti::type_id<T>()](podThatVisits.operator->());
	}
};

Reflector GetPodReflector(AssetPod* pod);