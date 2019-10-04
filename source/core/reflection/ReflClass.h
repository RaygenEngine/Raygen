#pragma once
#include "core/reflection/Property.h"
#include <string>
#include <vector>
#include <unordered_map>

// Object that describes a reflected class.
class ReflClass
{
	TypeId m_type;

	// PERF: Possible to use a constexpr array here.
	std::vector<Property> m_properties;

	// PERF: possible to constexpr hash variable names and use them in this map prehashed.
	std::unordered_map<std::string, size_t> m_hashTable;

	void AppendProperties(const ReflClass& other)
	{
		size_t index = m_properties.size();
		for (auto& prop : other.GetProperties())
		{
			m_properties.push_back(prop);
			m_hashTable[std::string(prop.GetName())] = index++;
		}
	}

	static const char* RemoveVariablePrefix(const char* name)
	{
		if (name[0] != 0 && name[1] != 0 && name[2] != 0 &&
			name[0] == 'm' && name[1] == '_')
		{
			return (name + 2);
		}
		return name;
	};

public:
	template<typename T>
	static ReflClass Generate()
	{
		ReflClass reflClass;
		reflClass.m_type = refl::GetId<T>();
		T::GenerateReflection(reflClass);
		return reflClass;
	}

	template<typename This, typename Parent>
	static ReflClass Generate()
	{
		// TODO: should avoid multiple generates here
		ReflClass reflClass;
		reflClass.m_type = refl::GetId<This>();
		This::GenerateReflection(reflClass);
		reflClass.AppendProperties(Parent::StaticClass());
		return reflClass;
	}

	template<typename T>
	[[nodiscard]]
	bool IsA() const
	{
		return refl::GetId<T>() == m_type;
	}

	// Grabs the compiletime type id of the reflected class
	[[nodiscard]] TypeId GetTypeId() const { return m_type; }

	// Grabs the compiletime type id of the reflected class
	[[nodiscard]] std::string_view GetName() const { return m_type.name().begin(); }
	[[nodiscard]] std::string GetNameStr() const { return m_type.name().str(); }

	// Always Prefer reflection macros
	// Never run ouside of T::GenerateReflection
	template<typename T>
	void AddProperty(size_t offset_of, const char* varname, PropertyFlags::Type flags) 
	{
		static_assert(refl::CanBeProperty<T>, "This type is not reflectable and cannot be a property.");

		const char* name = RemoveVariablePrefix(varname);
		size_t index = m_properties.size();
		m_properties.push_back(Property(refl::GetId<T>(), offset_of, name, flags));
		m_hashTable[std::string(name)] = index;
		if constexpr (std::is_enum_v<T>)
		{
			m_properties[index].MakeEnum<T>();
		}
	}

	[[nodiscard]]
	bool HasProperty(const std::string& name) const
	{
		return m_hashTable.find(name) != m_hashTable.end();
	}

	// Returns nullptr if property was not found.
	[[nodiscard]]
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



namespace refl
{
	// TODO: move this to the file containing both PodReflection + NodeReflection
	// Gets the reflclass object of T and is specialized properly for every reflectable T.
	// If you are unsure of how to get the ReflClass of an object ALWAYS USE THIS instead of static members/member functions.
	template<typename T>
	const ReflClass& GetClass(T* obj)
	{
		// TODO: Static assert this for T
		if constexpr (std::is_base_of_v<Node, T>)
		{
			// Virtual call to get the lowest reflector even if T == Node
			return obj->GetClass();
		}
		else if constexpr (std::is_base_of_v<AssetPod, T>)
		{
			if constexpr (std::is_same_v<AssetPod, T>)
			{
				static_assert(false, "Implement this by specialization");
			}
			else
			{
				return T::StaticClass();
			}
		}
		else
		{
			static_assert(false, "This object T is not reflected");
		}
	}
}
