#ifndef REFLECTION_H
#define REFLECTION_H

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
	String
};

static std::vector<std::string> PropertyTypeName = {
	"NONE",
	"Int",
	"Bool",
	"Float",
	"Vec3",
	"String"
};

// IsReflected to compile time check if a type can be reflected.
template<typename Type> 
constexpr bool IsReflected = false;
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

template<PropertyType Type>
struct TypeFromReflection { static_assert("Expected a value of the enum PropertyType."); };
template<> struct TypeFromReflection<PropertyType::Int> { using type = int32; };
template<> struct TypeFromReflection<PropertyType::Bool> { using type = bool; };
template<> struct TypeFromReflection<PropertyType::Float> { using type = float; };
template<> struct TypeFromReflection<PropertyType::Vec3> { using type = glm::vec3; };
template<> struct TypeFromReflection<PropertyType::String> { using type = std::string; };

#include "system/reflection/Property.h"
#include "system/reflection/Reflector.h"

// ALWAYS expected right at the beginning of a class declaration body
#define REFLECT(Class)	public: \
Reflector m_reflector = {#Class};  \
private:						  // Restore private specifier.

// ALWAYS expected to run in member function
#define REFLECT_VAR(Variable) GetReflector(this).AddProperty(#Variable, Variable);


#endif // REFLECTION_H
