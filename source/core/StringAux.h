#pragma once

#include <string>
#include <iterator>

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
