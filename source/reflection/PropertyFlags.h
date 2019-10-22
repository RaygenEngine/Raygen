#pragma once

namespace PropertyFlags {
using Type = uint64;

constexpr Type NoSave = (1 << 0);
constexpr Type NoLoad = (1 << 1);
constexpr Type NoCopy = (1 << 2);

constexpr Type NoEdit = (1 << 8);

// == NoSave | NoLoad | NoCopy, should probably be used everywhere instead of NoSave/NoLoad
constexpr Type Transient = NoSave | NoLoad | NoCopy;
// == NoSave | NoLoad | NoCopy | NoEdit, Usefull for stuff that is for debugging only and gets calculated constantly
constexpr Type Generated = NoSave | NoLoad | NoCopy | NoEdit;


// Only relevant for vec3 and vec4 types
constexpr Type Color = (1 << 9);

// Only relevant for std::string types
constexpr Type Multiline = (1 << 10);

// Only relevant for float types. (could be used for vec3 in the future)
constexpr Type Degrees = (1 << 11);


template<typename... Flags>
constexpr PropertyFlags::Type Pack(Flags... f)
{
	static_assert((std::is_same_v<PropertyFlags::Type, Flags> && ...));
	return (f | ...);
}

[[maybe_unused]] static PropertyFlags::Type Pack()
{
	return 0;
}
} // namespace PropertyFlags
