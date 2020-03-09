#pragma once

#include <string>

namespace str {

// Using this will also add support for C++20 heterogenous lookup: https://wg21.link/P0919R3
// (only MSVC > 19.23 at: 23/2/2020)

namespace detail {
	enum class CaseSensitivity
	{
		Insensitive,
		Sensitive
	};

	template<CaseSensitivity Sens = CaseSensitivity::Sensitive>
	struct EqualImpl {
		using is_transparent = void;
		static constexpr bool isInsensitive = Sens == CaseSensitivity::Insensitive;

		constexpr bool operator()(std::string_view lhs, std::string_view rhs) const { return compare(lhs, rhs); }

	private:
		static constexpr bool compareChar(char c1, char c2) noexcept
		{
			if constexpr (isInsensitive) {
				return c1 == c2 || std::tolower(c1) == std::tolower(c2);
			}
			return c1 == c2;
		}

		constexpr bool compare(std::string_view str1, std::string_view str2) const noexcept
		{
			return ((str1.size() == str2.size()) && std::equal(str1.begin(), str1.end(), str2.begin(), &compareChar));
		}
	};

	// Case insensitive only
	struct LessImpl {
		using is_transparent = void;

		struct LessComp {
			bool operator()(const unsigned char& c1, const unsigned char& c2) const
			{
				return tolower(c1) < tolower(c2);
			}
		};
		bool operator()(std::string_view s1, std::string_view s2) const noexcept
		{
			return std::lexicographical_compare(s1.begin(), s1.end(), s2.begin(), s2.end(), LessComp());
		}
	};

	template<CaseSensitivity Sens = CaseSensitivity::Sensitive>
	struct HashImpl {
		using transparent_key_equal = EqualImpl<Sens>;
		static constexpr bool isInsensitive = Sens == CaseSensitivity::Insensitive;

		constexpr size_t operator()(std::string_view txt) const noexcept { return hash(txt); }
		constexpr size_t operator()(const std::string& txt) const noexcept { return hash(txt); }
		constexpr size_t operator()(const char* txt) const noexcept { return hash(txt); }

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
} // namespace detail


// Case Sensitive
using Hash = detail::HashImpl<detail::CaseSensitivity::Sensitive>;
// Case Sensitive
using Equal = detail::EqualImpl<detail::CaseSensitivity::Sensitive>;

using HashInsensitive = detail::HashImpl<detail::CaseSensitivity::Insensitive>;
using EqualInsensitive = detail::EqualImpl<detail::CaseSensitivity::Insensitive>;
using LessInsensitive = detail::LessImpl;

// These "instances" are to be used as a function eg: str::hash("txt");
constexpr inline Hash hash;
constexpr inline Equal equal;
constexpr inline HashInsensitive hashInsensitive;
constexpr inline EqualInsensitive equalInsensitive;


template<typename T>
concept CCharSeq = true; // CHECK: string concept

// TEST:
template<CONC(CCharSeq) T>
std::vector<std::string_view> split(const T& str, std::string_view delims = " ")
{
	std::vector<std::string_view> output;
	for (auto first = str.data(), second = str.data(), last = first + str.size(); second != last && first != last;
		 first = second + 1) {
		second = std::find_first_of(first, last, std::cbegin(delims), std::cend(delims));

		if (first != second) {
			output.emplace_back(first, second - first);
		}
	}
	if (output.size() == 0) {
		output.emplace_back(str);
	}
	return output;
}

//
// Starts With Insensitive. (for sensitive use std)
//
constexpr inline bool startsWithInsensitive(std::string_view who, std::string_view startsWithTxt)
{
	return who.size() >= startsWithTxt.size()
		   && str::EqualInsensitive{}(who.substr(0, startsWithTxt.size()), startsWithTxt);
}


} // namespace str
