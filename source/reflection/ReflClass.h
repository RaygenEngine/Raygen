#pragma once

#include "reflection/Property.h"
#include "reflection/NodeFlags.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Node;

// Object that describes a reflected class.
class ReflClass {
	// PERF: constexpr the members, the generated properties, tables, and sets can be generated at compiletime
	TypeId m_type;

	std::vector<Property> m_properties;

	std::unordered_map<std::string, size_t> m_hashTable;

	const ReflClass* m_parentClass{ nullptr };

	std::unordered_set<const ReflClass*> m_childClasses;

	// Currently hardcoded for nodes only
	std::function<Node*()> m_createInstanceFunction{};

	NodeFlags::Type m_classFlags{ 0 };

	static constexpr char8 c_defaultIcon[] = FA_DOT_CIRCLE;
	// An "optional" icon for this class, as font character. Not the best place to be as it is "editor metadata" but not
	// as big of a deal to seperate this (yet)
	// The icon is supposed to be supported by the editor font.
	const char8* m_icon = c_defaultIcon;

	void AppendProperties(const ReflClass& other)
	{
		size_t index = m_properties.size();
		for (auto& prop : other.GetProperties()) {
			m_properties.push_back(prop);
			m_hashTable[std::string(prop.GetName())] = index++;
		}
		m_parentClass = &other;
		ReplaceIfDefaultIcon(other.m_icon);
	}

	void AddChildClass(const ReflClass* newChild) { m_childClasses.emplace(newChild); }

	[[nodiscard]] bool IsIconDefault() const
	{
		return std::u8string_view{ m_icon } == std::u8string_view{ c_defaultIcon };
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
	static ReflClass Generate(const char8* replaceIcon = c_defaultIcon)
	{
		ReflClass reflClass;
		reflClass.m_type = refl::GetId<T>();
		T::GenerateReflection(reflClass);
		if constexpr (std::is_base_of_v<Node, T> && !std::is_same_v<Node, T>) {
			reflClass.m_createInstanceFunction = &T::NewInstance;
		}
		reflClass.ReplaceIfDefaultIcon(replaceIcon);
		return reflClass;
	}

	template<typename This, typename Parent>
	static ReflClass Generate(ReflClass* thisLoc)
	{
		ReflClass reflClass;
		reflClass.m_type = refl::GetId<This>();
		This::GenerateReflection(reflClass);
		if constexpr (std::is_base_of_v<Node, This> && !std::is_same_v<Node, This>) {
			reflClass.m_createInstanceFunction = &This::NewInstance;
		}
		reflClass.AppendProperties(Parent::StaticClass());
		Parent::Z_MutableClass().AddChildClass(thisLoc);
		return reflClass;
	}

	// Returns true on EXACT type and not children of T.
	template<typename T>
	[[nodiscard]] bool IsExact() const
	{
		return refl::GetId<T>() == m_type;
	}

	// Returns true when the ReflClass instance represents a class that is derived from T AND NOT the same as T.
	// This call iterates the hierarchy for now and should be avoided for performance reasons. If we actually need
	// something like this every frame we should implement a parent set or array for each class in the ReflClass.
	template<typename T>
	[[nodiscard]] bool IsDerivedFrom() const
	{
		const ReflClass* thisCl = this;
		while (thisCl->m_parentClass != nullptr) {
			thisCl = thisCl->m_parentClass;
			if (thisCl->IsExact<T>()) {
				return true;
			}
		}
		return false;
	}


	// Attempts to create an instance of this class.
	// This will abort if HasCreateInstance returns false.
	template<typename T>
	[[nodiscard]] T* CreateInstance() const
	{
		CLOG_ABORT(!m_createInstanceFunction,
			"Attempting to create an instance of a ReflClass that has no instance function.");

		CLOG_ERROR(!IsA<T>(), "Attempting to create an instance of a ReflClass of incorrect type.");
		if (!IsA<T>()) {
			return nullptr;
		}
		return static_cast<T*>(m_createInstanceFunction());
	}

	[[nodiscard]] Node* CreateNodeInstance() const
	{
		CLOG_ABORT(!m_createInstanceFunction,
			"Attempting to create an instance of a ReflClass that has no instance function.");

		return m_createInstanceFunction();
	}

	[[nodiscard]] bool HasCreateInstance() const { return !(!m_createInstanceFunction); }


	// Grabs the compiletime type id of the reflected class
	[[nodiscard]] TypeId GetTypeId() const { return m_type; }

	// Grabs the compiletime type id of the reflected class
	[[nodiscard]] std::string_view GetName() const { return m_type.name(); }
	[[nodiscard]] std::string GetNameStr() const { return m_type.name_str(); }

	// Grabs the text icon for editor facing text of this type
	[[nodiscard]] const char8* GetIcon() const { return m_icon; }


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

	void AddFlags(NodeFlags::Type packedFlags) { m_classFlags |= packedFlags; }

	// True if ALL flags are found.
	[[nodiscard]] bool HasFlags(NodeFlags::Type flags) const { return ((m_classFlags & flags) == flags); }


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

	void SetIcon(const char8* icon) { m_icon = icon; }

	void ReplaceIfDefaultIcon(const char8* icon)
	{
		if (IsIconDefault()) {
			SetIcon(icon);
		}
	}


	[[nodiscard]] const ReflClass* GetParentClass() const { return m_parentClass; }

	[[nodiscard]] const std::vector<Property>& GetProperties() const { return m_properties; }

	[[nodiscard]] const std::unordered_set<const ReflClass*>& GetChildClasses() const { return m_childClasses; }

	// Assumes all classes objects are generated by macros.
	bool operator==(const ReflClass& other) const { return this == &other; }
	bool operator!=(const ReflClass& other) const { return !(*this == other); }
};
