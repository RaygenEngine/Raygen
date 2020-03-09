#pragma once

#include <string>

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

} // namespace str
