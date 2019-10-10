#pragma once

#include <vector>
#include <algorithm>

namespace utl {
template<class T>
[[nodiscard]] constexpr typename std::remove_reference<T>::type&& force_move(T const& t) noexcept = delete;

template<class T>
[[nodiscard]] constexpr typename std::remove_reference<T>::type&& force_move(T&& t) noexcept
{
	return std::move(t);
}
} // namespace utl
