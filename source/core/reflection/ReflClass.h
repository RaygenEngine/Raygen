#pragma once
#include "core/reflection/Property.h"
#include <string>
#include <vector>
#include <unordered_map>

// Object that describes a reflected class.
class ReflClass {
	TypeId m_type;

	// PERF: Possible to use a constexpr array here.
	std::vector<Property> m_properties;

	// PERF: possible to constexpr hash variable names and use them in this map prehashed.
	std::unordered_map<std::string, size_t> m_hashTable;

	const ReflClass* m_parentClass;

	ReflClass()
		: m_parentClass(nullptr)
	{
	}

	void AppendProperties(const ReflClass& other)
	{
		size_t index = m_properties.size();
		for (auto& prop : other.GetProperties()) {
			m_properties.push_back(prop);
			m_hashTable[std::string(prop.GetName())] = index++;
		}
		m_parentClass = &other;
	}

public:
	[[nodiscard]] static constexpr const char* RemoveVariablePrefix(const char* name)
	{
		if (name[0] != 0 && name[1] != 0 && name[2] != 0 && name[0] == 'm' && name[1] == '_') {
			return (name + 2);
		}
		return name;
	};

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
	[[nodiscard]] bool IsA() const
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
	Property& AddProperty(size_t offset_of, const char* varname, PropertyFlags::Type flags)
	{
		static_assert(refl::CanBeProperty<T>, "This type is not reflectable and cannot be a property.");

		const char* name = RemoveVariablePrefix(varname);
		size_t index = m_properties.size();
		m_properties.push_back(Property(refl::GetId<T>(), offset_of, name, flags));
		m_hashTable[std::string(name)] = index;
		if constexpr (std::is_enum_v<T>) {
			m_properties[index].MakeEnum<T>();
		}
		return m_properties[index];
	}

	[[nodiscard]] bool HasProperty(const std::string& name) const
	{
		return m_hashTable.find(name) != m_hashTable.end();
	}

	// Returns nullptr if property was not found.
	[[nodiscard]] const Property* GetPropertyByName(const std::string& name) const
	{
		auto it = m_hashTable.find(name);
		if (it == m_hashTable.end()) {
			return nullptr;
		}
		return &m_properties[it->second];
	}

	[[nodiscard]] const ReflClass* GetParentClass() const { return m_parentClass; }

	const std::vector<Property>& GetProperties() const { return m_properties; }
};
