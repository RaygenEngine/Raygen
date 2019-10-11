#pragma once
#include "reflection/ReflEnum.h"
#include "asset/AssetManager.h"
#include <nlohmann/json.hpp>

namespace glm {
inline void from_json(const nlohmann::json& j, glm::vec3& vec)
{
	auto it = j.find("x");
	if (it != j.end()) {
		vec.x = j.value<float>("x", 0.f);
		vec.y = j.value<float>("y", 0.f);
		vec.z = j.value<float>("z", 0.f);
		return;
	}
	// Search for rgb mode
	vec.x = j.value<float>("r", 0.f);
	vec.y = j.value<float>("g", 0.f);
	vec.z = j.value<float>("b", 0.f);
}
inline void from_json(const nlohmann::json& j, glm::vec4& vec)
{
	auto it = j.find("x");
	if (it != j.end()) {
		vec.x = j.value<float>("x", 0.f);
		vec.y = j.value<float>("y", 0.f);
		vec.z = j.value<float>("z", 0.f);
		vec.w = j.value<float>("w", 0.f);
		return;
	}

	vec.x = j.value<float>("r", 0.f);
	vec.y = j.value<float>("g", 0.f);
	vec.z = j.value<float>("b", 0.f);
	vec.z = j.value<float>("a", 0.f);
}
} // namespace glm
template<typename T>
inline void from_json(const nlohmann::json& j, PodHandle<T>& handle)
{
	if (j.is_string()) {
		handle = AssetManager::GetOrCreate<T>(j.get<std::string>())
	}
}
inline void from_json(const nlohmann::json& j, MetaEnumInst& enumInst)
{
	if (j.is_string()) {
		enumInst.SetValueByStr(j.get<std::string>());
	}
	else if (j.is_number_integer()) {
		enumInst.SetValue(j.get<enum_under_t>());
	}
}
