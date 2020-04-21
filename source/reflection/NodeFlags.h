#pragma once

namespace NodeFlags {
using Type = uint64;

constexpr Type NoUserCreated = (1 << 0);


template<typename... Flags>
constexpr NodeFlags::Type Pack(Flags... f)
{
	static_assert((std::is_same_v<NodeFlags::Type, Flags> && ...));
	return (f | ...);
}

[[maybe_unused]] static NodeFlags::Type Pack()
{
	return 0;
}
} // namespace NodeFlags
