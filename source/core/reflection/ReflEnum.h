#pragma once
#include "core/reflection/TypeId.h"
#define MAGIC_ENUM_RANGE_MIN -1
#define MAGIC_ENUM_RANGE_MAX 24
#include "magic_enum/magic_enum.hpp"
#include <unordered_map>
#include <map>
#include <functional>

using enum_under_t = int32_t;

namespace detail {
template<typename T>
std::function<void(void*, enum_under_t)> CreateGenericEnumSetter()
{
	static_assert(std::is_enum_v<T>, "Expects an enum type.");
	return [](void* enumPtr, enum_under_t value) {
		auto optV = magic_enum::enum_cast<T>(static_cast<magic_enum::underlying_type_t<T>>(value));
		(*static_cast<T*>(enumPtr)) = optV.value();
	};
}

template<typename T>
std::function<enum_under_t(void*)> CreateGenericEnumGetter()
{
	static_assert(std::is_enum_v<T>, "Expects an enum type.");
	return [](void* enumPtr) -> enum_under_t {
		T& inst = (*static_cast<T*>(enumPtr));
		return static_cast<enum_under_t>(magic_enum::enum_integer(inst));
	};
}
} // namespace detail

struct MetaEnumInst;

#define REFL_ENUM_RANGE(Enum, Min, Max)                                                                                \
	namespace magic_enum {                                                                                             \
		template<>                                                                                                     \
		struct enum_range<Enum> {                                                                                      \
			static constexpr int min = Min;                                                                            \
			static constexpr int max = Max;                                                                            \
		};                                                                                                             \
	}

//
// Enum Reflection for our reflection system.
// Multiple limitations should be noted here:
//
// * No value aliasing is supported, if you have aliased values check the workaround in Neargye's magic_enum docs.
// * Reflected values are only the values in the range [-1, 24], You can change this for your enums using the custom
// macro
//   REFL_ENUM_RANGE(MyEnum, Min, Max) right after your enum declaration. (big ranges -> bigger compile times)
// * Reflected values can be set in the int32_t range.
//
//

struct ReflEnum {
	using under_t = enum_under_t; // Universal underlying type used for reflection. (enum values)

	std::unordered_map<std::string, under_t> strToValue;
	std::map<under_t, std::string_view> valueToStr;

	// Setter function to avoid UB, the saved function here knows the actual underlying enum type.
	std::function<void(void*, under_t)> setter;
	std::function<under_t(void*)> getter;

	TypeId type;

	template<typename Enum>
	static ReflEnum FillProperties()
	{
		ReflEnum e;
		size_t count = magic_enum::enum_count<Enum>();

		e.type = refl::GetId<Enum>();

		e.strToValue.reserve(count);
		e.setter = detail::CreateGenericEnumSetter<Enum>();
		e.getter = detail::CreateGenericEnumGetter<Enum>();

		for (auto& [valueE, strView] : magic_enum::enum_entries<Enum>()) {
			under_t value = static_cast<under_t>(valueE);
			e.strToValue.insert(std::pair<std::string, under_t>(strView, value));
			e.valueToStr.insert({ value, strView });
		}

		return e;
	}

public:
	template<typename Enum>
	static const ReflEnum& GetMeta()
	{
		static_assert(std::is_enum_v<Enum>, "Attempted to generate a non enum.");
		static ReflEnum e = FillProperties<Enum>();
		return e;
	}

	[[nodiscard]] const std::map<under_t, std::string_view>& GetValues() const { return valueToStr; }

	[[nodiscard]] TypeId GetType() const { return type; }

	[[nodiscard]] const std::unordered_map<std::string, under_t>& GetStringsToValues() const { return strToValue; }

	template<typename T>
	[[nodiscard]] MetaEnumInst TieEnum(T& obj) const
	{
		return Make(&obj, *this, refl::GetId<T>())
	}

	friend struct MetaEnumInst;
};

// The tie object remains valid for as long as the tied instnace does not get moved and
// directly safely memory edits the enum instance.
struct MetaEnumInst {
	using under_t = enum_under_t;

private:
	friend struct ReflEnum;
	friend class Property;
	void* obj;
	const ReflEnum& meta;
	TypeId type;
	MetaEnumInst(void* inObj, const ReflEnum& inMeta, TypeId inId)
		: obj(inObj)
		, meta(inMeta)
		, type(inId)
	{
	}

	static MetaEnumInst Make(void* obj, const ReflEnum& inMeta, TypeId objType)
	{
		CLOG_ASSERT(inMeta.type != objType,
			"GetRefEnum did not encounter correct object type. Types where: Enum: {} | Given: {}", inMeta.type.name(),
			objType.name());
		return MetaEnumInst(obj, inMeta, objType);
	}

public:
	[[nodiscard]] TypeId GetType() const { return type; }
	[[nodiscard]] const ReflEnum& GetEnum() const { return meta; }

	// The value will only go through if it is a valid value for this enum.
	void SetValue(under_t value)
	{
		if (meta.valueToStr.find(value) != meta.valueToStr.end()) {
			meta.setter(obj, value);
		}
	}

	[[nodiscard]] under_t GetValue() const { return meta.getter(obj); }

	// Gets the string of the enum value. Expects that
	[[nodiscard]] std::string_view GetValueStr() const
	{
		under_t value = meta.getter(obj);
		return meta.valueToStr.at(value);
	}

	// Attempts to set the value by the string that matches the enum.
	void SetValueByStr(std::string str) const
	{
		auto it = meta.strToValue.find(str);
		if (it == meta.strToValue.end()) {
			return;
		}
		meta.setter(obj, it->second);
	}

	// Safely gets a pointer to the enum object. (or nullptr if types don't match)
	// In most actuall cases where you would know this type, there are probably better ways to do this.
	template<typename T>
	[[nodiscard]] T* GetVerified()
	{
		static_assert(std::is_enum_v<T>, "T is not an enum. This call would always fail");
		if (refl::GetId<T>() != type) {
			return nullptr;
		}
		return static_cast<T*>(obj);
	}
};