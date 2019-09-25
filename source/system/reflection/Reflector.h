#pragma once
#include "system/reflection/Property.h"

class Reflector
{
protected:
	std::string m_objectName;
	std::vector<ExactProperty> m_properties;
	std::unordered_map<std::string, size_t> m_hashTable;

	friend class StaticReflector;
public:
	void SetName(const std::string& name) { m_objectName = name; }
	std::string GetName() const { return m_objectName; }

	std::vector<ExactProperty>& GetProperties() { return m_properties; }
	
	bool HasProperty(const std::string& name) 
	{
		return m_hashTable.find(name) != m_hashTable.end();
	}
	
	// Cost O(1) due to internal hashing
	// Crashes if property does not exist. Use GetPropertyByName to avoid this.
	ExactProperty& GetPropertyByNameUnsafe(const std::string& name) { return m_properties[m_hashTable[name]]; }

	// Returns nullptr if property does not exist
	ExactProperty* GetPropertyByName(const std::string& name)
	{
		if (HasProperty(name))
		{
			return &GetPropertyByNameUnsafe(name);
		}
		return nullptr;
	}

	template<typename Type>
	ExactProperty& AddProperty(std::string&& name, Type& variable)
	{
		static_assert(IsReflected<Type>, "This type is not supported for reflection.");

		std::string copy = name;

		const size_t insertIndex = m_properties.size();
		ExactProperty inserted = ExactProperty::MakeProperty(variable, name);
		
		m_hashTable.insert({ std::move(copy), insertIndex });
		return m_properties.emplace_back(inserted);
	}

	// Internal version to remove m_ prefix from variables added through the macro
	template<typename Type>
	ExactProperty& AutoAddProperty(char* name, Type& variable)
	{
		static_assert(IsReflected<Type>, "This type is not supported for reflection.");
		if (name[0] != 0 && name[1] != 0 && name[2] != 0 && name[0] == 'm' && name[1] == '_')
		{
			name = (name + 2);
		}
		return AddProperty(std::string(name), variable);
	}


	Reflector(std::string name = "NoName")
		: m_objectName(name)
	{}

};

// Static reflector contains properties that are "offsetof" from the object pointer instead of actual heap values.
class StaticReflector 
{
	std::string m_className;
	std::vector<OffsetProperty> m_offsetProperties;
	std::unordered_map<std::string, size_t> m_hashTable;
public:
	bool m_generated{ false };

	StaticReflector(char* name)
		: m_className(name)
	{}
	

	template<typename Type>
	OffsetProperty& AutoAddProperty(char* name, size_t offset_of)
	{
		static_assert(IsReflected<Type>, "This type is not supported for reflection.");
		if (name[0] != 0 && name[1] != 0 && name[2] != 0 && name[0] == 'm' && name[1] == '_')
		{
			name = (name + 2);
		}
		return AddProperty<Type>(std::string(name), offset_of);
	}


	template<typename Type>
	OffsetProperty& AddProperty(std::string&& name, size_t offset_of)
	{
		static_assert(IsReflected<Type>, "This type is not supported for reflection.");

		std::string copy = name;

		const size_t insertIndex = m_offsetProperties.size();
		OffsetProperty inserted = OffsetProperty::MakeProperty<Type>(offset_of, name);

		m_hashTable.insert({ std::move(copy), insertIndex });
		return m_offsetProperties.emplace_back(inserted);
	}

	// This Reflector object contains valid data for as long as 'instance' is not Moved Or Deleted.
	// Otherwise accessing / calling any function on the return value will result in undefined behaviour
	template<typename Obj>
	Reflector ToAbsoluteReflector(Obj* instance) const
	{
		static_assert(HasStaticReflector<Obj>, "Object is of incorrect type. This object type is not statically reflected.");

		Reflector r;
		r.m_hashTable = m_hashTable;
		r.SetName(m_className);
		r.m_properties.reserve(m_offsetProperties.size());
		StaticReflector* self = const_cast<StaticReflector*>(this);
		for (auto& originalProp : self->m_offsetProperties)
		{
			r.m_properties.push_back(utl::force_move(originalProp.ToExactProperty(instance)));
		}
		return utl::force_move(r);
	}
	
};

// Abstracts over the multiple underlying reflector types for inspection purposes.
class ReflectorViewer
{
private:
	size_t ptr_offset;

public:
	std::string GetName() const 
	{

	}

	const std::vector<ExactProperty>& GetProperties() 
	{ 

	}

	bool HasProperty(const std::string& name)
	{

	}

};


//template<typename Class, typename = void>
//void GetReflector(Class* object)
//{
//	static_assert(false,
//				  "This is not a reflectable object."
//				  "Neither m_reflector nor StaticReflect was found"
//				  );
//}


template<typename T, typename = typename std::enable_if_t<!HasReflection<T>>>
void GetReflector(T* object)
{
	static_assert(false, "Not a reflected type");
}



template<typename ReflectedClass, typename = typename std::enable_if_t<HasMemberReflector<ReflectedClass>>>
auto GetReflector(ReflectedClass* object) -> Reflector&
{
	//static_assert(std::is_base_of_v<Reflector, decltype(object->m_reflector)>,
	//				"This object's reflector is an incorrect type."
	//			  );
	return object->m_reflector;
}


template<typename ReflectedClass, typename = typename std::enable_if_t<HasStaticReflector<ReflectedClass>>>
auto GetReflector(ReflectedClass* object) -> Reflector
{
	return utl::force_move(ReflectedClass::StaticReflect().ToAbsoluteReflector(object));
}
