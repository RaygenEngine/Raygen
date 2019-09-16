#pragma once
#include "system/reflection/Reflection.h"

namespace PropertyFlags
{
	using Type = uint64;
	
	constexpr Type NoSave	= (1 << 0);
	constexpr Type NoLoad	= (1 << 1);

	// == NoSave | NoLoad, should probably be used everywhere instead of NoSave/NoLoad
	constexpr Type Transient = NoSave | NoLoad;

	constexpr Type NoEdit	= (1 << 2);
	constexpr Type Color	= (1 << 3);
	constexpr Type Multiline = (1 << 4);

	template<typename ...Flags>
	constexpr PropertyFlags::Type Pack(Flags ...f)
	{
		static_assert((std::is_same_v<PropertyFlags::Type, Flags> && ...));
		return (f | ...);
	}

	static PropertyFlags::Type Pack()
	{
		return 0;
	}
}

// a generic property that can be of any type.
class Property
{
protected:
	PropertyType m_type;
	PropertyFlags::Type m_flags;
	std::string m_name;

public:
	const std::string& GetName() { return m_name;  }
	
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

	
	// Sets all flags to 0 then adds the given flags.
	Property& InitFlags(PropertyFlags::Type flags)
	{
		m_flags = flags;
		return *this;
	}

	// Adds the given flags.
	Property& AddFlags(PropertyFlags::Type flags)
	{
		m_flags |= flags;
		return *this;
	}

	// Removes the given flags.
	Property& RemoveFlags(PropertyFlags::Type flags)
	{
		flags = ~flags;
		m_flags &= flags;
		return *this;
	}

	// 
	bool HasFlags(PropertyFlags::Type flags) const
	{
		return ((m_flags & flags) == flags);
	}
};

// Points to a heap ptr directly
class ExactProperty : public Property
{
	// As noted this is a direct pointer to the memory of the mapped object.
	void* m_ptr;

	friend class OffsetProperty;

	static ExactProperty MakePropertyUnsafe(void* realVariableMemory, PropertyType type, const std::string& name)
	{
		ExactProperty created;
		created.m_type = type;
		created.m_ptr = realVariableMemory;
		created.m_name = name;

		return utl::force_move(created);
	}

public:
	// Calls one depending on the type of the property. Forwards a type& as a single lambda param. (eg: int& PropValue)
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

		if constexpr (!std::is_void_v<return_type_t<IntF>>)
		{
			return {};
		}
	}


	// ALWAYS only request types you are sure are the correct type with IsA() first.
	// this WILL have undefined behavior if you try to convert to a different type.
	template<typename As>
	As& GetRef()
	{
		static_assert(IsReflected<As> && !std::is_base_of_v<Reflector, As>,
					  "This type is not supported for reflection and the conversion will always fail.");

		assert(IsA<As>());
		return (*reinterpret_cast<As*>(m_ptr));
	}

	template<typename Type>
	static ExactProperty MakeProperty(Type& variable, const std::string& name)
	{
		static_assert(IsReflected<Type>, "This type is not supported for reflection.");

		ExactProperty created;
		created.m_type = ReflectionFromType<Type>;
		created.m_ptr = &variable;
		created.m_name = name;

		return created;
	}
};

// Points to a heap ptr directly
class OffsetProperty : public Property
{
	// As noted this is a direct pointer to the memory of the mapped object.
	size_t m_offset;

	void* GetRealMemoryAddr(void* objInstance)
	{
		byte* ptr = static_cast<byte*>(objInstance);
		return ptr + m_offset;
	}

public:
	// ALWAYS only request types you are sure are the correct type with IsA() first.
	// this WILL have undefined behavior if you try to convert to a different type.
	template<typename As, typename Obj>
	As& GetRef(Obj* obj)
	{
		static_assert(IsReflected<As> && !std::is_base_of_v<Reflector, As>,
					  "This type is not supported for reflection and the conversion will always fail.");

		static_assert(IsStaticallyReflected<Obj>,
					  "Object type is not static reflected. This is probably a wrong object.");


		assert(IsA<As>());
		return (*reinterpret_cast<As*>(GetRealMemoryAddr(obj)));
	}
	
	// DONT EVER STORE this unless you know what you are doing.
	// If the underlying staticly reflected obj instance gets moved EVER this will be an invalid ExactProperty.
	// If the object is actually staticly reflected in the engine, it is probably because its MOVABLE or has unknown lifetime.
	template<typename Obj>
	ExactProperty ToExactProperty(Obj* obj)
	{
		static_assert(HasStaticReflector<Obj>,
					  "Object type is not static reflected. This is probably a wrong object.");

		return utl::force_move(ExactProperty::MakePropertyUnsafe(GetRealMemoryAddr(obj), m_type, m_name));
	}

	// This will not detect the type automatically.
	// Avoid using directly to prevent type errors.
	template<typename Type>
	static OffsetProperty MakeProperty(size_t offset_of, const std::string& name)
	{
		static_assert(IsReflected<Type>, "This type is not supported for reflection.");

		OffsetProperty created;
		created.m_type = ReflectionFromType<Type>;
		created.m_offset = offset_of;
		created.m_name = name;

		return created;
	}
};