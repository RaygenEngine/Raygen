#pragma once

#include "reflection/ReflClass.h"
#include "reflection/PodReflection.h"
#include "reflection/GetClass.h"
#include "asset/util/ParsingAux.h"
#include <type_traits>

namespace refltools {
using json = nlohmann::json;

namespace detail {
	DECLARE_HAS_FUNCTION_DETECTOR(Begin);
	DECLARE_HAS_FUNCTION_DETECTOR(End);

	DECLARE_HAS_FUNCTION_DETECTOR(PreProperty);
	DECLARE_HAS_FUNCTION_DETECTOR(PostProperty);
} // namespace detail

// These are all the possible functions and their documentation for the visitor
struct ReflClassVisitor {
	void Begin(const ReflClass&) {} // Pre Begin iteration on reflector's properties
	void End(const ReflClass&) {}   // Post End iteration on reflector's properties

	// Pre Call visitor on proprety. If return type is boolean then returning false will skip visiting for this property
	bool PreProperty(const Property&) { return true; }
	void PostProperty(const Property&) {} // Post Call visitor on proprety

	template<typename T>
	void operator()(T&, const Property&)
	{
	} // Overload this for T.

	// Overload that would catch all enum properties. Enum properties do not visit on the operator() with their type.
	void operator()(MetaEnumInst&, const Property&) {}
};

static_assert(detail::HasPreProperty<ReflClassVisitor>::value, "Reflection tools test failed.");

namespace detail {
	template<typename T, typename Visitor>
	bool VisitIf(Visitor& v, const Property& p, void* obj)
	{
		if (p.IsA<T>()) {
			v.operator()(p.GetRef<T>(obj), p);
			return true;
		}
		return false;
	}

	template<typename Visitor, typename... Types>
	bool MaybeVisit(Visitor& v, const Property& p, void* obj)
	{
		return (VisitIf<Types, Visitor>(v, p, obj) || ...); // Expand all ...Types and run visit.
	}

	template<typename Visitor, typename... Types>
	bool MaybeVisit_WrapVector(Visitor& v, const Property& p, void* obj)
	{
		return (VisitIf<std::vector<Types>, Visitor>(v, p, obj) || ...);
	}

	template<typename Visitor, typename... Types>
	bool MaybeVisit_WrapPodHandle(Visitor& v, const Property& p, void* obj)
	{
		return (VisitIf<PodHandle<Types>, Visitor>(v, p, obj) || ...);
	}

	template<typename Visitor, typename... Types>
	bool MaybeVisit_WrapVectorPodHandle(Visitor& v, const Property& p, void* obj)
	{
		return (VisitIf<std::vector<PodHandle<Types>>, Visitor>(v, p, obj) || ...);
	}
} // namespace detail

// This is where all the instantiations of the sub types happen.
// If you are working on adding more types and the macro lists don't cut it, you should also include them here
template<typename Visitor>
void CallVisitorOnProperty(Visitor& v, const Property& p, void* obj)
{
	if (p.IsEnum()) {
		auto EnumInst = p.GetEnumRef(obj);
		v.operator()(EnumInst, p); // Keeping as a local allows overloaded operator()(T& ...) to pass enums through.
		return;
	}
	[[maybe_unused]] bool cc = detail::MaybeVisit<Visitor, Z_REFL_TYPES>(v, p, obj)
							   || detail::MaybeVisit_WrapPodHandle<Visitor, Z_POD_TYPES>(v, p, obj)
							   || detail::MaybeVisit_WrapVectorPodHandle<Visitor, Z_POD_TYPES>(v, p, obj);
}

template<typename ReflectedObj, typename Visitor>
void CallVisitorOnEveryProperty(ReflectedObj* obj, Visitor& v)
{
	using namespace detail;

	const ReflClass& cl = refl::GetClass(obj);

	if constexpr (HasBegin<Visitor>::value) { // NOLINT
		v.Begin(cl);
	}

	for (auto& p : cl.GetProperties()) {
		if constexpr (HasPreProperty<Visitor>::value) {                                           // NOLINT
			if constexpr (std::is_same_v<return_type_t<decltype(&Visitor::PreProperty)>, bool>) { // NOLINT
				if (!v.PreProperty(p)) {
					continue;
				}
			}
			else {
				v.PreProperty(p);
			}
		}

		CallVisitorOnProperty(v, p, obj);

		if constexpr (HasPostProperty<Visitor>::value) { // NOLINT
			v.PostProperty(p);
		}
	}

	if constexpr (HasEnd<Visitor>::value) { // NOLINT
		v.End(cl);
	}
}

struct ReflClassOperationResult {
	// Properties with same name but different types are included here:
	size_t PropertiesNotFoundInDestination{ 0 };


	size_t PropertiesNotFoundInSource{ 0 };

	// Counts type miss matches where variable name is the same
	// eg: this counts:
	// - SRC: bool myInt;
	// - DST: int32 myInt;
	size_t TypeMissmatches{ 0 };

	// Counts flag missmatches on type matches
	// eg: this counts:
	// - SRC: int32 myInt; [flag: NoSave]
	// - DST: int32 myInt; [flag: NoLoad]
	//
	// eg: this doesnt:
	// - SRC: bool myInt; [flag: NoSave]
	// - DST: int32 myInt; [flag: NoLoad]
	size_t FlagMissmatches{ 0 };

	bool IsExactlyCorrect()
	{
		return PropertiesNotFoundInDestination == 0 && PropertiesNotFoundInSource == 0 && TypeMissmatches == 0
			   && FlagMissmatches == 0;
	}
};

namespace detail {
	struct CopyIntoVisitor {
		CopyIntoVisitor(const ReflClass* inDstClass, void* dst)
			: dstClass(inDstClass)
			, dstObj(dst)
		{
			operationResult.PropertiesNotFoundInSource = dstClass->GetProperties().size();
		}

		const ReflClass* dstClass;
		void* dstObj;

		ReflClassOperationResult operationResult;

		const Property* GetMatch(const Property& prop)
		{
			auto dstProp = dstClass->GetPropertyByName(prop.GetNameStr());
			if (!dstProp) {
				operationResult.PropertiesNotFoundInDestination++;
				return nullptr;
			}

			if (dstProp->GetType() != prop.GetType()) {
				operationResult.TypeMissmatches++;
				return nullptr;
			}

			if (!dstProp->HasSameFlags(prop)) {
				operationResult.FlagMissmatches++;
			}

			operationResult.PropertiesNotFoundInSource--;
			return dstProp;
		}


		bool PreProperty(const Property& p)
		{
			if (p.HasFlags(PropertyFlags::NoCopy)) {
				GetMatch(p);
				return false;
			}
			return true;
		}

		template<typename T>
		void operator()(T& ref, const Property& prop)
		{
			auto dstProp = GetMatch(prop);
			if (!dstProp) {
				return;
			}
			dstProp->GetRef<T>(dstObj) = ref;
		}

		void operator()(MetaEnumInst& ref, const Property& prop)
		{
			auto dstProp = GetMatch(prop); // guaranteed to return enum or nullptr
			if (!dstProp) {
				return;
			}
			dstProp->GetEnumRef(dstObj).SetValue(ref.GetValue());
		}
	};
} // namespace detail

// Copies values from 2 reflected objects all ReflClass properties from src dst
template<typename SrcT, typename DestT>
ReflClassOperationResult CopyClassTo(SrcT* src, DestT* dst)
{
	detail::CopyIntoVisitor visitor(&refl::GetClass(dst), dst);
	CallVisitorOnEveryProperty(src, visitor);
	return visitor.operationResult;
}


struct JsonToPropVisitor {

	const json& j;
	PropertyFlags::Type flagsToSkip;

	JsonToPropVisitor(const json& inJson, PropertyFlags::Type inFlagsToSkip = PropertyFlags::NoLoad)
		: j(inJson)
		, flagsToSkip(inFlagsToSkip)
	{
	}

	bool PreProperty(const Property& p)
	{
		if (p.HasFlags(flagsToSkip)) {
			return false;
		}
		return true;
	}

	template<typename T>
	void operator()(T& v, const Property& prop)
	{
		auto it = j.find(prop.GetName());
		if (it != j.end()) {
			it->get_to<T>(v);
		}
	}
};

// With Parent path override and conditionally gltfPreload
struct JsonToPropVisitor_WithRelativePath {

	JsonToPropVisitor parentVisitor;
	BasePodHandle podParent;
	bool gltfPreload;

	JsonToPropVisitor_WithRelativePath(const json& inJson, BasePodHandle inParent, bool inGltfPreload = false,
		PropertyFlags::Type inFlagsToSkip = PropertyFlags::NoLoad)
		: parentVisitor(inJson, inFlagsToSkip)
		, podParent(inParent)
		, gltfPreload(inGltfPreload)
	{
	}

	// forward any non ovreloaded call to parent
	template<typename T>
	void operator()(T& v, const Property& prop)
	{
		parentVisitor.operator()(v, prop);
	}

	template<typename T>
	PodHandle<T> LoadHandle(const std::string& str)
	{
		return AssetManager::GetOrCreateFromParent<T>(str, podParent);
	}

	template<>
	PodHandle<ModelPod> LoadHandle(const std::string& str)
	{
		auto handle = AssetManager::GetOrCreateFromParent<ModelPod>(str, podParent);
		if (gltfPreload && str.find(".gltf") != std::string::npos) {
			AssetManager::PreloadGltf(AssetManager::GetPodUri(handle) + "{}");
		}
		return handle;
	}

	template<typename T>
	void operator()(PodHandle<T>& p, const Property& prop)
	{
		auto it = parentVisitor.j.find(prop.GetName());
		if (it == parentVisitor.j.end()) {
			LOG_ABORT("Failed to find pod property: {}", prop.GetName());
		}
		p = LoadHandle<T>(it->get<std::string>());
	}

	template<typename T>
	void operator()(std::vector<PodHandle<T>>& v, const Property& prop)
	{
		auto it = parentVisitor.j.find(prop.GetName());
		if (it != parentVisitor.j.end() && it->is_array()) {
			for (auto& elem : *it) {
				v.emplace_back(LoadHandle<T>(elem.get<std::string>()));
			}
		}
	}
};

struct ToJsonVisitor {
	json& j;
	ToJsonVisitor(json& inJson)
		: j(inJson)
	{
	}

	bool PreProperty(const Property& p) { return !p.HasFlags(PropertyFlags::NoSave); }

	template<typename T>
	void operator()(T& ref, const Property& p)
	{
		j[p.GetNameStr()] = ref;
	}
};


// Assigns each property to the given json using default to_json functions,
// Skips all NoSave properties
template<typename ReflectedObj>
void PropertiesToJson(ReflectedObj* obj, nlohmann::json& j)
{
	ToJsonVisitor visitor(j);
	CallVisitorOnEveryProperty(obj, visitor);
}

} // namespace refltools
