#pragma once

#include <sstream>
#include <iterator>

namespace utl {
inline std::string Repeat(const std::string& input, size_t num)
{
	std::ostringstream os;
	std::fill_n(std::ostream_iterator<std::string>(os), num, input);
	return os.str();
}

template<typename T>
inline bool SplitStringIntoTArray(T* arr, int arrSize, const std::string& str, char delimiter)
{
	bool success = false;
	std::stringstream ss(str);
	T value;
	auto i = 0;
	while (ss >> value) {
		success = i == (arrSize - 1);
		if (i > arrSize - 1)
			break;
		arr[i++] = value;
		if (ss.peek() == delimiter)
			ss.ignore();
	}
	return success;
}

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

inline std::string UnnamedDescription(const std::string& str)
{
	return str.empty() ? "<unnamed>" : str;
}
} // namespace utl

inline std::string operator*(std::string str, std::size_t n)
{
	return utl::Repeat(str, n);
}
