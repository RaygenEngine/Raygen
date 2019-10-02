#pragma once
#include "core/reflection/TypeId.h"
#include "core/reflection/PropertyFlags.h"
#include <string_view>

// a generic property that can be of any type.
class Property
{
	// Only class that is allowed to "construct" properties.
	friend class ReflClass;
protected:
	TypeId m_type;
	PropertyFlags::Type m_flags;
	std::string_view m_name;

	// Result of offsetof. It is size_t in bytes from the beginning of an object.
	size_t m_offset;

	// Returns real memory address from the offsetof for a specific instance.
	void* GetRealMemoryAddr(void* objInstance) const
	{
		byte* ptr = static_cast<byte*>(objInstance);
		return ptr + m_offset;
	}

	bool IsA(TypeId inType) const
	{
		return m_type == inType;
	}

	Property(TypeId type, size_t offset, std::string_view name, PropertyFlags::Type flags)
		: m_type(type)
		, m_flags(flags)
		, m_name(name)
		, m_offset(offset) {}

public:
	std::string_view GetName() const { return m_name; }

	// Check if this property is of this type.
	template<typename T>
	bool IsA() const
	{
		static_assert(CanBeProperty<T>, "This check will always fail. T cannot be a reflected property.");
		return refl::GetId<T>() == m_type;
	}

	// Returns a reference to the underlying variable of the passed object instance.
	// This will assert if the requested type is incorrect.
	// TODO: SourceT MUST be the type this property was created from.
	template<typename T, typename SourceT>
	T& GetRef(SourceT* obj) const
	{
		static_assert(CanBeProperty<T>, "This will always fail. T is not a reflected property.");
		static_assert(IsReflected<SourceT>(), "Source obj is  not a reflected object. This will always fail.");
		
		CLOG_ASSERT(IsA<T>(), "Requested variable '{}' from class '{}' as '{}'. Actual type was: '{}' ", GetName(), refl::GetName<SourceT>(), m_type.name());
		
		return *static_cast<T*>(GetRealMemoryAddr(obj));
	}

	// True if ALL flags are found.
	bool HasFlags(PropertyFlags::Type flags) const
	{
		return ((m_flags & flags) == flags);
	}

	// True only when the flags match exactly
	bool HasSameFlags(const Property& other) const
	{
		return other.m_flags == m_flags;
	}
};