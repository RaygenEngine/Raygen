#pragma once

#include <string_view>

namespace mti {
#if defined(__clang__)
#	define MTI_PRETTY_FUNC __PRETTY_FUNCTION__
#	define MTI_LEFT_CHOP   25 // "auto mti::GetName() [T = "
#	define MTI_RIGHT_CHOP  2  // "]\0"
#elif defined(__GNUC__)
#	define MTI_PRETTY_FUNC __PRETTY_FUNCTION__
#	define MTI_LEFT_CHOP   40 // "constexpr auto mti::GetName() [with T = "
#	define MTI_RIGHT_CHOP  2  // "]\0"
#elif defined(_MSC_VER)
#	define MTI_PRETTY_FUNC __FUNCSIG__
#	define MTI_LEFT_CHOP   26 // "auto __cdecl mti::GetName<"
#	define MTI_RIGHT_CHOP  17 // ">(void) noexcept\0"
#else
#	error "MTI: Unsupported compiler."
#endif

#define MTI_GET_VIEW                                                                                                   \
	{                                                                                                                  \
		MTI_PRETTY_FUNC + MTI_LEFT_CHOP, sizeof(MTI_PRETTY_FUNC) - (MTI_LEFT_CHOP + MTI_RIGHT_CHOP)                    \
	}

namespace detail {
	constexpr std::string_view LeftChop(std::string_view v, std::size_t len)
	{
		return std::string_view{ v.data() + len, v.size() - len };
	}

	constexpr std::string_view FilterView(std::string_view v) noexcept
	{
#if defined(_MSC_VER)
		switch (*v.data()) {
			case 's': return LeftChop(v, 7); // "struct "
			case 'c': return LeftChop(v, 6); // "class "
			case 'e': return LeftChop(v, 5); // "enum "
			case 'u': return LeftChop(v, 6); // "union "
		}
		return v;
#elif defined(__GNUC__) || defined(__clang__)
		return v;
#endif
	};

	constexpr size_t StrHash(const std::string_view str) noexcept
	{
		static_assert(sizeof(size_t) == 8);
		// FNV-1a 64 bit algorithm
		size_t result = 0xcbf29ce484222325; // FNV offset basis
		for (char c : str) {
			result ^= c;
			result *= 1099511628211; // FNV prime
		}
		return result;
	}
} // namespace detail

struct TypeId {
	constexpr TypeId(std::string_view name) noexcept
		: name_{ name }
		, hash_{ detail::StrHash(name) }
	{
	}

	constexpr TypeId() noexcept
		: TypeId{ "void" }
	{
	}

	constexpr std::size_t hash() const noexcept { return hash_; }

	constexpr std::string_view name() const noexcept { return name_; }

	std::string name_str() const noexcept { return std::string{ name_ }; }

	friend constexpr bool operator==(const TypeId& lhs, const TypeId& rhs) noexcept { return lhs.hash_ == rhs.hash_; }

	friend constexpr bool operator!=(const TypeId& lhs, const TypeId& rhs) noexcept { return !(lhs == rhs); }

private:
	std::string_view name_;
	std::size_t hash_;
};

template<typename T>
constexpr auto GetName() noexcept
{
	return detail::FilterView(MTI_GET_VIEW);
}

template<typename T>
constexpr auto GetHash() noexcept
{
	return detail::StrHash(GetName<T>());
}

template<typename T>
constexpr auto GetTypeId() noexcept
{
	return TypeId(GetName<T>());
}

} // namespace mti

using TypeId = mti::TypeId;

namespace refl {
template<typename T>
constexpr TypeId GetId()
{
	return mti::GetTypeId<T>();
}

template<typename T>
constexpr std::string_view GetName()
{
	return mti::GetName<T>();
}
} // namespace refl
