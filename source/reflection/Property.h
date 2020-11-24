#pragma once

#include "reflection/PropertyFlags.h"
#include "reflection/PropertyTypes.h"
#include "reflection/ReflEnum.h"
#include "reflection/TypeId.h"

#include <string_view>
#include <variant>

struct SerializedProperty {
	std::string type;
	std::string name;
	PropertyFlags::Type flags;
	size_t offset_of;

	template<typename Archive>
	void serialize(Archive& ar)
	{
		ar(type, name, flags, offset_of);
	}
};

// a generic property that can be of any type.
class Property {
	// Only class that is allowed to "construct" properties.
	friend class ReflClass;
	friend class RuntimeClass;


protected:
	TypeId m_type;
	PropertyFlags::Type m_flags;
	std::string_view m_name;

	// Result of offsetof. It is size_t in bytes from the beginning of an object.
	size_t m_offset;

	// Relevant only if the property is an enum.
	// Non owning pointer to the static ReflEnum structure generated for this enum class
	const ReflEnum* m_enum{ nullptr };


	float m_editorSpeed{ 1.f };
	float m_editorMin{ 0.f };
	float m_editorMax{ std::numeric_limits<float>::max() };

	// Returns real memory address from the offsetof for a specific instance.
	void* GetRealMemoryAddr(void* objInstance) const
	{
		byte* ptr = static_cast<byte*>(objInstance);
		return ptr + m_offset;
	}

	[[nodiscard]] bool IsA(TypeId inType) const { return m_type == inType; }

	Property(TypeId type, size_t offset, std::string_view name, PropertyFlags::Type flags)
		: m_type(type)
		, m_flags(flags)
		, m_name(name)
		, m_offset(offset)
	{
	}

	template<typename Enum>
	void MakeEnum()
	{
		static_assert(std::is_enum_v<Enum>, "Make enum expects an enum type");
		m_enum = &ReflEnum::GetMeta<Enum>();
	}

public:
	SerializedProperty GenerateSerializedProperty() const
	{
		SerializedProperty serial;
		serial.type = m_type.name();
		serial.name = m_name;
		serial.flags = m_flags;
		serial.offset_of = m_offset;
		CLOG_ABORT(m_enum != nullptr, "Attempting to serialize enum property, unsupported yet.");
		return serial;
	}

public:
	[[nodiscard]] std::string_view GetName() const { return m_name; }
	[[nodiscard]] std::string GetNameStr() const { return std::string(m_name); }
	[[nodiscard]] TypeId GetType() const { return m_type; }
	[[nodiscard]] const ReflEnum* GetEnum() const { return m_enum; }
	[[nodiscard]] bool IsEnum() const { return m_enum != nullptr; }


	// Check if this property is of this type.
	template<typename T>
	[[nodiscard]] bool IsA() const
	{
		static_assert(refl::CanBeProperty<T>, "This check will always fail. T cannot be a reflected property.");
		return refl::GetId<T>() == m_type;
	}

	// Returns a reference to the underlying variable of the passed object instance.
	// This will assert if the requested type is incorrect.
	template<typename T>
	[[nodiscard]] T& GetRef(void* obj) const
	{
		static_assert(refl::CanBeProperty<T>, "This will always fail. T is not a reflected property.");

		CLOG_ABORT(!IsA<T>(), "Requested variable '{}' as '{}'. Actual type was: '{}' ", GetName(), refl::GetName<T>(),
			m_type.name());

		return *static_cast<T*>(GetRealMemoryAddr(obj));
	}

	[[nodiscard]] MetaEnumInst GetEnumRef(void* obj) const
	{
		CLOG_ABORT(!IsEnum(), "Requested GetRefEnum on a property that was not an enum: {}, type: {} ", GetName(),
			m_type.name());
		return MetaEnumInst::Make(GetRealMemoryAddr(obj), *GetEnum(), m_type);
	}

	// True if ALL flags are found.
	[[nodiscard]] bool HasFlags(PropertyFlags::Type flags) const { return ((m_flags & flags) == flags); }

	// True only when the flags match exactly
	[[nodiscard]] bool HasSameFlags(const Property& other) const { return other.m_flags == m_flags; }


public:
	//
	// Editor Meta Data stuff
	//
	[[nodiscard]] float Editor_GetSpeed() const { return m_editorSpeed; }

	// This will be float or T
	template<typename T>
	using Editor_MinMaxRetType = std::conditional_t<std::is_integral_v<T>, T, float>;

	template<typename T>
	[[nodiscard]] Editor_MinMaxRetType<T> Editor_GetMin() const
	{
		if constexpr (std::is_integral_v<T>) {
			return static_cast<T>(m_editorMin);
		}
		return m_editorMin;
	}

	template<typename T>
	[[nodiscard]] Editor_MinMaxRetType<T> Editor_GetMax() const
	{
		if (m_editorMax == std::numeric_limits<float>::max()) {
			if constexpr (std::is_same_v<decltype(std::numeric_limits<T>::max()), T>) {
				if constexpr (std::is_integral_v<T>) {
					return std::numeric_limits<T>::max();
				}
			}
		}
		if constexpr (std::is_integral_v<T>) {
			return static_cast<T>(m_editorMax);
		}
		return m_editorMax;
	}

	Property& Clamp(float min = 0.f, float max = std::numeric_limits<float>::max())
	{
		m_flags |= PropertyFlags::EditorClamp;
		m_editorMax = max;
		m_editorMin = min;
		return *this;
	}
};
