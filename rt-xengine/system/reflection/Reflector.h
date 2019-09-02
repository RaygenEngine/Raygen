#pragma once
#include "system/reflection/Property.h"

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