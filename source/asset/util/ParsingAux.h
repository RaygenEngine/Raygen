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
inline void to_json(nlohmann::json& j, const glm::vec3& p)
{
	j = nlohmann::json{ { "x", p.x }, { "y", p.y }, { "z", p.z } };
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

inline void to_json(nlohmann::json& j, const glm::vec4& p)
{
	j = nlohmann::json{ { "x", p.x }, { "y", p.y }, { "z", p.z }, { "w", p.w } };
}
} // namespace glm


template<typename T>
inline void from_json(const nlohmann::json& j, PodHandle<T>& handle)
{
	if (j.is_string()) {
		handle = AssetManager::GetOrCreate<T>(j.get<std::string>())
	}
}

template<typename T>
inline void to_json(nlohmann::json& j, const PodHandle<T>& handle)
{
	j = AssetManager::GetPodUri(handle);
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

inline void to_json(nlohmann::json& j, const MetaEnumInst& enumInst)
{
	j = enumInst.GetValueStr();
}

// contains Scene convention for general variable fields.
namespace sceneconv {
inline std::string FilterNodeClassName(std::string_view v)
{
	constexpr std::string_view filter = "Node";
	if (v.substr(v.size() - 4) == filter) {
		v = v.substr(0, v.size() - 4);
	}
	return smath::ToLower(std::string{ v });
}

inline const std::string typeLabel = "+type";
inline const std::string nameLabel = "+name";
inline const std::string trsLabel = "+trs";
inline const std::string childrenLabel = "~children";

// trs convention
inline const std::string posLabel = "pos";
inline const std::string rotLabel = "rot";
inline const std::string scaleLabel = "scale";
inline const std::string lookatLabel = "lookat";
} // namespace sceneconv
