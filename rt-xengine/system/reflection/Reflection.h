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

// a generic property that can be of any type.
struct Property
{
protected:
	PropertyType m_type;

	// As noted this is a direct pointer to the memory of the mapped object.
	void* m_ptr;

public:
	bool IsA(PropertyType type) 
	{
		return type == m_type;
	}

	template<typename Type>
	bool IsA()
	{
		return IsA(ReflectionFromType<Type>);
	}

	// ALWAYS only request types you are sure are the correct type with IsA() first.
	// this WILL have undefined behavior if you try to convert to a different type.
	template<typename As>
	As& GetRef() 
	{
		static_assert(IsReflected<As>, 
					  "This type is not supported for reflection and the conversion will always fail.");
		
		assert(IsA<As>());
		return (*reinterpret_cast<As*>(m_ptr));
	}

	template<typename Type>
	static Property MakeProperty(Type& Variable)
	{
		static_assert(IsReflected<Type>, "This type is not supported for reflection.");

		Property created;
		created.m_type = ReflectionFromType<Type>;
		created.m_ptr = &Variable;
		return created;
	}

	template<typename IntF, typename BoolF, typename FloatF, typename Vec3F, typename StringF>
	auto SwitchOnType(IntF Int, BoolF Bool, FloatF Float, Vec3F Vec3, StringF String) -> return_type_t<IntF>
	{
		switch (m_type)
		{
		case PropertyType::Int:
			return Int(GetRef<int32>());
			break;
		case PropertyType::Bool:
			return Bool(GetRef<bool>());
			break;
		case PropertyType::Float:
			return Float(GetRef<float>());
			break;
		case PropertyType::Vec3:
			return Vec3(GetRef<glm::vec3>());
			break;
		case PropertyType::String:
			return String(GetRef<std::string>());
			break;
		}

		if constexpr (!std::is_void_v<return_type_t<IntF>>) {
			return {};
		}
	}
};


// Each reflected Object should have an instance of this and return it if quered for ::GetReflector()
class Reflector 
{
protected:
	std::string m_objectName;
	std::unordered_map<std::string, Property> m_properties;

public:
	void SetName(const std::string& name) { m_objectName = name; }
	std::string GetName() const { return m_objectName; }
	
	// TODO: Dangerous non const ref until we implement more getters / setters for the map
	std::unordered_map<std::string, Property>& GetProperties() { return m_properties; }

	template<typename Type>
	void AddProperty(const std::string& name, Type& variable)
	{
		static_assert(IsReflected<Type>, "This type is not supported for reflection.");
		m_properties.insert({ name, Property::MakeProperty(variable) });
	}

	// Move Version
	template<typename Type>
	void AddProperty(std::string&& name, Type& variable)
	{
		static_assert(IsReflected<Type>, "This type is not supported for reflection.");
		m_properties.insert({ name, Property::MakeProperty(variable) });
	}

	Reflector(std::string name = "NoName")
		: m_objectName(name) 
	{}

};

template <typename T, typename = int>
struct HasReflector : std::false_type { };

template <typename T>
struct HasReflector<T, decltype((void)T::m_reflector, 0)> : std::true_type { };

template<typename ReflectedClass>
Reflector& GetReflector(ReflectedClass* object) 
{
	static_assert(HasReflector<ReflectedClass>::value,
				  "This class is not reflected."
				  "If you are trying to setup a class for reflection use the Reflect macros"
				  );

	static_assert(std::is_same_v<decltype(object->m_reflector), Reflector>,
				  "This object's reflector is an incorrect type."
				  "If you are trying to setup a class for reflection use the Reflect macros"
				  );

	return object->m_reflector;
}

// ALWAYS expected right at the beginning of a class declaration body
#define REFLECT(Class)	public: \
Reflector m_reflector = {#Class};  \
private:						  // Restore private specifier.

// ALWAYS expected to run in member function
#define REFLECT_VAR(Variable) GetReflector(this).AddProperty(#Variable, Variable);


#endif // REFLECTION_H
