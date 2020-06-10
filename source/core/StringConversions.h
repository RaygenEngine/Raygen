#pragma once

#include <string>
#include <iomanip>
#include <sstream>

namespace str {
template<typename T>
std::string toStr(T& value)
{
	return std::to_string(value);
}

template<typename T>
T fromStr(std::string str)
{
	T value;
	std::istringstream(str) >> value;
	return value;
}


// PERF: not more performant, for now copies string
template<typename T>
T fromStrView(std::string_view view)
{
	T value;
	std::istringstream(std::string(view)) >> value;
	return value;
}


template<>
inline bool fromStrView<bool>(std::string_view view)
{
	bool value;
	std::istringstream(std::string(view)) >> std::boolalpha >> value;
	return value;
}

// True when the view starts with ^[0-9]
inline bool startsWithNum(std::string_view view)
{
	if (view.size() == 0) {
		return false;
	}
	return (view[0] >= '0' && view[0] <= '9');
}
} // namespace str
