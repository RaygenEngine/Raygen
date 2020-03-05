#pragma once

#include <string>


namespace conv {
template<typename T>
std::string ToStr(T& value)
{
	return std::to_string(value);
}

template<typename T>
T FromStr(std::string str)
{
	T value;
	std::istringstream(str) >> value;
	return value;
}


// PERF: not more performant, for now copies string
template<typename T>
T FromStrView(std::string_view view)
{
	T value;
	std::istringstream(std::string(view)) >> value;
	return value;
}

} // namespace conv
