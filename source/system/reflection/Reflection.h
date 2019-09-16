#pragma once

// Basic reflection system based on "concepts" and template stuff
// 
// Objects that are reflected are supposed to contain a Reflector class variable
// 


#include "GLM/glm.hpp"
#include <unordered_map>
#include <vector>
#include <string>
#include "core/auxiliary/MetaTemplates.h"
#include <type_traits>


class Reflector;

// A reflection system that uses offsetof would be better but for simplicity we just initialize the reflector
// for every instance of the objects we want to reflect. Therefore we can save pointers directly.

// All the currently supported reflected types.
enum class PropertyType
{
	NONE = 0,
	Int = 1,
	Bool,
	Float,
	Vec3,
	String,
	AssetPtr,
};

static std::vector<std::string> PropertyTypeName = {
	"NONE",
	"Int",
	"Bool",
	"Float",
	"Vec3",
	"String",
	"AssetPtr"
};

class StaticReflector;
class Reflector;

namespace impl
{
template<class T, class Sig, class = void>struct has_static_reflector :std::false_type {};

template<class T, class R, class...Args>
struct has_static_reflector <T, R(Args...),
	std::enable_if_t< // std:: in C++1y
	std::is_convertible<
	decltype(T::StaticReflect(std::declval<Args>()...)),
	R
	>::value
	&& !std::is_same<R, void>::value
	>
> : std::true_type {};

template<class T, class...Args>
struct has_static_reflector <T, void(Args...),
	decltype(void(T::foo(std::declval<Args>()...)))
> : std::true_type {};

template<typename T> struct has_reflector {
	struct Fallback { Reflector m_reflector; }; // introduce member name "x"
	struct Derived : T, Fallback { };

	template<typename C, C> struct ChT;

	template<typename C> static char(&f(ChT<Reflector Fallback::*, &C::m_reflector>*))[1];
	template<typename C> static char(&f(...))[2];

	static bool const value = sizeof(f<Derived>(0)) == 2;
};
}


template<typename T>
constexpr bool HasMemberReflector = impl::has_reflector<T>::value;

template<typename T>
constexpr bool HasStaticReflector = impl::has_static_reflector<T, const StaticReflector&()>::value;

template<typename T>
constexpr bool HasReflection = HasMemberReflector<T> || HasStaticReflector<T>;


// IsReflected to compile time check if a type can be reflected.
template<typename Type> 
constexpr bool IsReflected = HasAssetReflector<std::remove_pointer_t<Type>>;
template<> constexpr bool IsReflected<int32> = true;
template<> constexpr bool IsReflected<bool> = true;
template<> constexpr bool IsReflected<float> = true;
template<> constexpr bool IsReflected<glm::vec3> = true;
template<> constexpr bool IsReflected<std::string> = true;


template<typename Type>
constexpr PropertyType ReflectionFromType = PropertyType::NONE;
template<> PropertyType ReflectionFromType<int32> = PropertyType::Int;
template<> PropertyType ReflectionFromType<bool> = PropertyType::Bool;
template<> PropertyType ReflectionFromType<float> = PropertyType::Float;
template<> PropertyType ReflectionFromType<glm::vec3> = PropertyType::Vec3;
template<> PropertyType ReflectionFromType<std::string> = PropertyType::String;

//template<PropertyType Type>
//struct TypeFromReflection { static_assert("Expected a value of the enum PropertyType."); };
//template<> struct TypeFromReflection<PropertyType::Int> { using type = int32; };
//template<> struct TypeFromReflection<PropertyType::Bool> { using type = bool; };
//template<> struct TypeFromReflection<PropertyType::Float> { using type = float; };
//template<> struct TypeFromReflection<PropertyType::Vec3> { using type = glm::vec3; };
//template<> struct TypeFromReflection<PropertyType::String> { using type = std::string; };
//template<> struct TypeFromReflection<PropertyType::ReflectorOwner> { using type = Reflector; };



// ALWAYS expected to run in member function
#define REFLECT_VAR(Variable, ...) GetReflector(this).AutoAddProperty(#Variable, Variable).InitFlags(PropertyFlags::Pack(__VA_ARGS__));


// Static reflection, this macro leaves the scope on private:
#define STATIC_REFLECTOR(Class)	private:			\
using __ThisType = Class; public:					\
[[nodiscard]]										\
static const StaticReflector& StaticReflect()		\
{													\
	static StaticReflector reflector("Class");		\
	if (!reflector.m_generated)						\
	{												\
		FillMembers(reflector);						\
		reflector.m_generated = true;				\
	}												\
	return reflector;								\
} private:											\
static void FillMembers(StaticReflector& reflector)	

// Static reflect var
#define S_REFLECT_VAR(Variable, ...)				\
	reflector.AutoAddProperty<decltype(Variable)>(#Variable, offsetof(__ThisType, Variable)).InitFlags(PropertyFlags::Pack(__VA_ARGS__));



