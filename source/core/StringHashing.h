#pragma once

#include <string>
#include <string_view>

// provides case insensitive hashing

namespace str {

// Using this will also add support for C++20 heterogenous lookup: https://wg21.link/P0919R3
// (only MSVC > 19.23 at: 23/2/2020)


enum class CaseSensitivity
{
	Insensitive,
	Sensitive
};

template<CaseSensitivity Sens = CaseSensitivity::Sensitive>
struct EqualImpl {
	using is_transparent = void;
	static constexpr bool isInsensitive = Sens == CaseSensitivity::Insensitive;

	bool operator()(std::string_view lhs, std::string_view rhs) const { return copmare(lhs, rhs); }

private:
	static bool compareChar(char c1, char c2) noexcept
	{
		if constexpr (isInsensitive) {
			return c1 == c2 || std::tolower(c1) == std::tolower(c2);
		}
		return c1 == c2;
	}

	bool copmare(std::string_view str1, std::string_view str2) const noexcept
	{
		return ((str1.size() == str2.size()) && std::equal(str1.begin(), str1.end(), str2.begin(), &compareChar));
	}
};

template<CaseSensitivity Sens = CaseSensitivity::Sensitive>
struct HashImpl {
	using transparent_key_equal = EqualImpl<Sens>;
	static constexpr bool isInsensitive = Sens == CaseSensitivity::Insensitive;

	size_t operator()(std::string_view txt) const noexcept { return hash(txt); }
	size_t operator()(const std::string& txt) const noexcept { return hash(txt); }
	size_t operator()(const char* txt) const noexcept { return hash(txt); }

private:
	constexpr size_t hash(std::string_view view) const noexcept
	{
		static_assert(sizeof(size_t) == 8);
		// FNV-1a 64 bit algorithm
		size_t result = 0xcbf29ce484222325; // FNV offset basis
		for (char c : view) {
			if constexpr (isInsensitive) {
				result ^= static_cast<char>(std::tolower(c));
			}
			else {
				result ^= c;
			}
			result *= 1099511628211; // FNV prime
		}
		return result;
	};
};

// Case Sensitive
using Hash = HashImpl<CaseSensitivity::Sensitive>;
// Case Sensitive
using Equal = EqualImpl<CaseSensitivity::Sensitive>;

using HashInsensitive = HashImpl<CaseSensitivity::Insensitive>;
using EqualInsensitive = EqualImpl<CaseSensitivity::Insensitive>;


} // namespace str
