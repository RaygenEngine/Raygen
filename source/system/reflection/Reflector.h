#pragma once
#include "system/reflection/Property.h"

// Each reflected Object should have an instance of this and return it if quered for ::GetReflector()
class Reflector
{
protected:
	std::string m_objectName;
	std::vector<Property> m_properties;
	// PERF: this should probably be string_view
	std::unordered_map<std::string, size_t> m_hashTable;

public:
	void SetName(const std::string& name) { m_objectName = name; }
	std::string GetName() const { return m_objectName; }

	
	std::vector<Property>& GetProperties() { return m_properties; }

	
	bool HasProperty(const std::string& name) 
	{
		return m_hashTable.find(name) != m_hashTable.end();
	}
	
	// Cost O(1) due to internal hashing
	// Crashes if property does not exist. Use GetPropertyByName to avoid this.
	Property& GetPropertyByNameUnsafe(const std::string& name) { return m_properties[m_hashTable[name]]; }

	// Returns nullptr if property does not exist
	Property* GetPropertyByName(const std::string& name)
	{
		if (HasProperty(name))
		{
			return &GetPropertyByNameUnsafe(name);
		}
		return nullptr;
	}

	
	template<typename Type>
	Property& AddProperty(std::string&& name, Type& variable)
	{
		static_assert(IsReflected<Type>, "This type is not supported for reflection.");

		std::string copy = name;

		const size_t insertIndex = m_properties.size();
		Property inserted = Property::MakeProperty(variable, name);
		
		m_hashTable.insert({ std::move(copy), insertIndex });
		return m_properties.emplace_back(inserted);
	}

	// Internal version to remove m_ prefix from variables added through the macro
	template<typename Type>
	Property& AutoAddProperty(char* name, Type& variable)
	{
		static_assert(IsReflected<Type>, "This type is not supported for reflection.");
		name = (name + 2);
		return AddProperty(std::string(name), variable);
	}


	Reflector(std::string name = "NoName")
		: m_objectName(name)
	{}

};

template<typename ReflectedClass>
Reflector& GetReflector(ReflectedClass* object)
{
	static_assert(HasReflector<ReflectedClass>,
				  "This class is not reflected."
				  "If you are trying to setup a class for reflection use the Reflect macros"
				  );

	static_assert(std::is_base_of_v<Reflector, decltype(object->m_reflector)>,
				  "This object's reflector is an incorrect type."
				  "If you are trying to setup a class for reflection use the Reflect macros"
				  );

	return object->m_reflector;
}

class AssetReflector : public Reflector
{

};

class PodReflector : public Reflector
{

};