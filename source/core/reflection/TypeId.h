#pragma once
#include "ctti/type_id.hpp"

using TypeId = ctti::type_id_t;

namespace refl
{
	template<typename T>
	constexpr TypeId GetId() 
	{
		return ctti::type_id<T>();
	}

	template<typename T>
	constexpr std::string_view GetName()
	{
		auto cview = ctti::nameof<T>();
		return std::string_view(cview.begin(), cview.size());
	}
}
