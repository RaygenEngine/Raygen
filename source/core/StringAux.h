#pragma once

#include <string>

// TODO: rename
namespace smath {

inline void LTrim(std::string& s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) { return !std::isspace(c); }));
}

inline void RTrim(std::string& s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int c) { return !std::isspace(c); }).base(), s.end());
}

inline void Trim(std::string& s)
{
	LTrim(s);
	RTrim(s);
}

inline bool CaseInsensitiveCompare(const std::string& a, const std::string& b)
{
	// if different sizes, they are different
	if (a.size() != b.size()) {
		return false;
	}

	return std::equal(
		a.begin(), a.end(), b.begin(), b.end(), [](char c1, char c2) { return tolower(c1) == tolower(c2); });
}

inline std::string ToLower(const std::string& str)
{
	std::string temp = str;
	std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
	return temp;
}

} // namespace smath

namespace str {
// TODO: string concept
template<typename T>
std::vector<std::string_view> Split(T str, std::string_view delims = " ")
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
} // namespace str
