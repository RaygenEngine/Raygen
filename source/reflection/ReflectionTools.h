#pragma once

#include "assets/PodIncludes.h"
#include "assets/util/ParsingUtl.h"
#include "reflection/GetClass.h"
#include "reflection/PodReflection.h"
#include "reflection/ReflClass.h"

#include <fstream>
#include <sstream>
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
	void Begin(void* objectPtr, const ReflClass&) {} // Pre Begin iteration on reflector's properties
	void End(const ReflClass&) {}                    // Post End iteration on reflector's properties

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
							   || detail::MaybeVisit_WrapVector<Visitor, Z_REFL_TYPES>(v, p, obj)
							   || detail::MaybeVisit_WrapPodHandle<Visitor, ENGINE_POD_TYPES>(v, p, obj)
							   || detail::MaybeVisit_WrapVectorPodHandle<Visitor, ENGINE_POD_TYPES>(v, p, obj);
}


// Extended version of CallVisitorOnEveryProperty that Accepts RuntimeClass types.
template<typename Visitor>
void CallVisitorOnEveryPropertyEx(void* obj, const ReflClass& cl, Visitor& v)
{
	// CHECK: Duplicate code, (calling the normal function would cause type safety loss)
	using namespace detail;

	if constexpr (HasBegin<Visitor>::value) {
		v.Begin(obj, cl);
	}

	for (auto& p : cl.GetProperties()) {
		if constexpr (HasPreProperty<Visitor>::value) {
			if constexpr (std::is_same_v<return_type_t<decltype(&Visitor::PreProperty)>, bool>) {
				if (!v.PreProperty(p)) {
					continue;
				}
			}
			else {
				v.PreProperty(p);
			}
		}

		CallVisitorOnProperty(v, p, obj);

		if constexpr (HasPostProperty<Visitor>::value) {
			v.PostProperty(p);
		}
	}

	if constexpr (HasEnd<Visitor>::value) {
		v.End(cl);
	}
}

// Visitor must implement "template<VarType> void operator()(VarType& value, const Property& property)"
template<typename ReflectedObj, typename Visitor>
void CallVisitorOnEveryProperty(ReflectedObj* obj, Visitor& v)
{
	using namespace detail;

	const ReflClass& cl = refl::GetClass(obj);

	if constexpr (HasBegin<Visitor>::value) {
		v.Begin(obj, cl);
	}

	for (auto& p : cl.GetProperties()) {
		if constexpr (HasPreProperty<Visitor>::value) {
			if constexpr (std::is_same_v<return_type_t<decltype(&Visitor::PreProperty)>, bool>) {
				if (!v.PreProperty(p)) {
					continue;
				}
			}
			else {
				v.PreProperty(p);
			}
		}

		CallVisitorOnProperty(v, p, obj);

		if constexpr (HasPostProperty<Visitor>::value) {
			v.PostProperty(p);
		}
	}

	if constexpr (HasEnd<Visitor>::value) {
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

// Copies values from 2 reflected objects all ReflClass properties from src dst, supports runtime typeless objects
inline ReflClassOperationResult CopyClassToEx(
	void* src, void* dst, const ReflClass& srcClass, const ReflClass& dstClass)
{
	detail::CopyIntoVisitor visitor(&dstClass, dst);
	CallVisitorOnEveryPropertyEx(src, srcClass, visitor);
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

namespace detail {
	constexpr const char* c_importHintSuffix = "_importHint_";
}


// With Parent path override and conditionally gltfPreload
struct JsonToPropVisitor_WorldLoad {

	JsonToPropVisitor parentVisitor;

	JsonToPropVisitor_WorldLoad(const json& inJson, PropertyFlags::Type inFlagsToSkip = PropertyFlags::NoLoad)
		: parentVisitor(inJson, inFlagsToSkip)
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
		return AssetRegistry::GetAsyncHandle<T>(str);
	}

	template<typename T>
	void operator()(PodHandle<T>& p, const Property& prop)
	{
		auto it = parentVisitor.j.find(prop.GetName());
		if (it != parentVisitor.j.end()) {
			p = LoadHandle<T>(it->get<std::string>());
		}
		else {
			LOG_WARN(
				"Failed to find pod property: {} (Saved with older engine version?) Using default pod or "
				"\"{}\".",
				prop.GetName(), detail::c_importHintSuffix);
		}

		if (p.IsDefault()) {
			auto it = parentVisitor.j.find(prop.GetNameStr() + detail::c_importHintSuffix);
			if (it != parentVisitor.j.end()) {
				p = AssetManager->ImportAs<T>(fs::path(it->get<std::string>()), true);
				if (!p.IsDefault()) {
					LOG_INFO("Imported {} through the import hint.", it->get<std::string>());
				}
			}
		}
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

	template<typename T>
	void operator()(PodHandle<T>& ref, const Property& p)
	{
		j[p.GetNameStr()] = ref;
		j[p.GetNameStr() + detail::c_importHintSuffix]
			= fs::relative(fs::path(AssetRegistry::GetPodImportPath(ref))).generic_string();
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

template<typename ReflectedObj>
bool ToJsonFile(ReflectedObj* obj, const fs::path& file)
{
	static_assert(
		!std::is_const_v<ReflectedObj>, "This cannot be const. Please only const_cast if you absolutely HAVE to.");
	std::ofstream fstr(file);
	if (!fstr) {
		return false;
	}
	json j;
	ToJsonVisitor visitor(j);
	CallVisitorOnEveryProperty(obj, visitor);
	fstr << j;
	return true;
}
//
template<typename ReflectedObj>
void JsonToProperties(ReflectedObj* obj, nlohmann::json& j)
{
	JsonToPropVisitor visitor(j);
	CallVisitorOnEveryProperty(obj, visitor);
}


template<typename ReflectedObj>
void FromJsonFile(ReflectedObj* obj, const fs::path& file)
{
	std::ifstream fstr(file);
	if (!fstr) {
		return; // could probably create it
	}
	auto j = json::parse(fstr, nullptr, false);
	JsonToPropVisitor visitor(j);
	CallVisitorOnEveryProperty(obj, visitor);
}


struct ToStringVisitor {
	std::string& str;
	std::ostringstream os;
	ToStringVisitor(std::string& instr)
		: str(instr)
	{
	}

	template<typename T>
	void operator()(T& ref, const Property& p)
	{
		os << p.GetName() << ": " << ref << '\n';
	}

	template<typename T>
	void operator()(std::vector<T>& ref, const Property& p)
	{
		os << p.GetName() << ": ";
		for (const auto& r : ref) {
			os << "\t" << r << ",\n";
		}
	}

	void operator()(std::string& ref, const Property& p)
	{
		os << p.GetName() << ": ";
		if (p.HasFlags(PropertyFlags::Multiline)) {
			os << "\n=========\n" << ref << "\n==========\n";
		}
		else {
			os << '"' << ref << '"' << '\n';
		}
	}

	void operator()(MetaEnumInst& ref, const Property& p) { os << p.GetName() << ": " << ref.GetValueStr() << '\n'; }

	template<typename T>
	void operator()(PodHandle<T>& ref, const Property& p)
	{
		os << p.GetName() << ": " << AssetRegistry::GetPodUri(ref) << '\n';
	}

	template<typename T>
	void operator()(std::vector<PodHandle<T>>& ref, const Property& p)
	{
		os << p.GetName() << ":\n";
		for (auto& r : ref) {
			os << "\t" << AssetRegistry::GetPodUri(r) << ",\n";
		}
	}

	void operator()(glm::vec3& ref, const Property& p)
	{
		os << p.GetName() << ": " << ref.x << ", " << ref.y << ", " << ref.z << '\n';
	}

	void operator()(glm::vec4& ref, const Property& p)
	{
		os << p.GetName() << ": " << ref.x << ", " << ref.y << ", " << ref.z << ", " << ref.w << '\n';
	}

	void operator()(glm::mat4& ref, const Property& p)
	{
		os << p.GetName() << ": ";
		for (int32 i = 0; i < 4; ++i) {
			os << '\n';
			for (int32 j = 0; j < 4; ++j) {
				os << std::setprecision(4) << ref[i][j] << ", ";
			}
		}
		os << '\n';
	}
	void End(const ReflClass& r) { str += os.str(); }
};

// Assigns each property to the given json using default to_json functions,
// Skips all NoSave properties
template<typename ReflectedObj>
std::string PropertiesToText(ReflectedObj* obj)
{
	std::string t;
	ToStringVisitor visitor(t);
	CallVisitorOnEveryProperty(obj, visitor);
	return t;
}

} // namespace refltools


inline std::ostream& operator<<(std::ostream& os, const glm::vec3& ref)
{
	os << "[" << ref.x << ", " << ref.y << ", " << ref.z << "]";
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const glm::vec4& ref)
{
	os << "[" << ref.x << ", " << ref.y << ", " << ref.z << ", " << ref.w << "]";
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const glm::mat4& ref)
{
	for (int32 i = 0; i < 4; ++i) {
		os << '\n';
		for (int32 j = 0; j < 4; ++j) {
			os << std::setprecision(4) << ref[i][j] << ", ";
		}
	}
	os << '\n';
	return os;
}
