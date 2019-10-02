#pragma once
#include "core/reflection/Property.h"
#include "system/reflection/Reflection.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace detail
{
	const char* RemoveVariablePrefix(const char* name)
	{
		if (name[0] != 0 && name[1] != 0 && name[2] != 0 && 
			name[0] == 'm' && name[1] == '_')
		{
			return (name + 2);
		}
		return name;
	};
}

// Object that describes a reflected class.
class ReflClass
{
	TypeId m_type;

	// PERF: Possible to use a constexpr array here.
	std::vector<Property> m_properties;

	// PERF: possible to constexpr hash variable names and use them in this map prehashed.
	std::unordered_map<std::string, size_t> m_hashTable;

public:
	template<typename T>
	static ReflClass Generate()
	{
		ReflClass reflClass;
		reflClass.m_type = refl::GetId<T>();
		T::GenerateReflection(reflClass);
		return reflClass;
	}

	// Prefer reflection macros
	// Never run ouside of T::GenerateReflection
	template<typename T>
	void AddProperty(size_t offset_of, const char* varname, PropertyFlags::Type) 
	{
		static_assert(refl::CanBeProperty<T>(), "This type is not reflectable and cannot be a property.");

		const char* name = detail::RemoveVariablePrefix(varname);
		size_t index = m_properties.size();
		m_properties(Property(refl::GetId<T>(), offset_of, name, flags))
		m_hashTable[std::string(name)] = index;
	}

	// Grabs the compiletime type id of the reflected class
	TypeId GetTypeId() const { return m_type; }

	bool HasProperty(const std::string& name) const
	{
		return m_hashTable.find(name) != m_hashTable.end();
	}

	// Returns nullptr if property was not found.
	const Property* GetPropertyByName(const std::string& name) const
	{
		auto it = m_hashTable.find(name);
		if (it == m_hashTable.end())
		{
			return nullptr;
		}
		return &m_properties[it->second];
	}

	const std::vector<Property>& GetProperties() const { return m_properties; }
};

class Node;
class AssetPod;

template<typename T>
const ReflClass& GetClass(T* obj) 
{ 
	if constexpr (std::is_base_of_v<Node, T>)
	{
		// Virtual call to get the lowest reflector even if T == Node
		return obj->GetClass();
	}
	else if constexpr (std::is_base_of_v<AssetPod, T>)
	{
		if constexpr (std::is_same_v<AssetPod, T>)
		{
			static_assert("Implement");
		}
		else
		{
			return T::GetClass();
		}
	}
	else
	{
		static_assert("This object T is not reflected");
	}
}

