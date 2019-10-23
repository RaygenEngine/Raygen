#pragma once

#include "reflection/ReflEnum.h"
#include "asset/AssetManager.h"

#include <nlohmann/json.hpp>

namespace parsingaux {
// Default is taken by copy and deduces the template argument
template<typename T>
inline T SelectFromAlias(const nlohmann::json& j, const char* first, const char* second, T Default)
{
	auto it = j.find(first);
	if (it != j.end()) {
		return it->get<float>();
	}
	return j.value<float>(second, Default);
}
} // namespace parsingaux
namespace glm {
inline void from_json(const nlohmann::json& j, glm::vec3& vec)
{
	vec.x = ::parsingaux::SelectFromAlias(j, "x", "r", 0.0f);
	vec.y = ::parsingaux::SelectFromAlias(j, "y", "g", 0.0f);
	vec.z = ::parsingaux::SelectFromAlias(j, "z", "b", 0.0f);
}
inline void to_json(nlohmann::json& j, const glm::vec3& p)
{
	j = nlohmann::json{ { "x", p.x }, { "y", p.y }, { "z", p.z } };
}

inline void from_json(const nlohmann::json& j, glm::vec4& vec)
{
	vec.x = ::parsingaux::SelectFromAlias(j, "x", "r", 0.0f);
	vec.y = ::parsingaux::SelectFromAlias(j, "y", "g", 0.0f);
	vec.z = ::parsingaux::SelectFromAlias(j, "z", "b", 0.0f);
	vec.w = ::parsingaux::SelectFromAlias(j, "w", "a", 1.0f);
}

inline void to_json(nlohmann::json& j, const glm::vec4& p)
{
	j = nlohmann::json{ { "x", p.x }, { "y", p.y }, { "z", p.z }, { "w", p.w } };
}

inline void from_json(const nlohmann::json& js, glm::mat4& mat)
{
	for (int32 i = 0; i < 4; ++i) {
		for (int32 j = 0; j < 4; ++j) {
			mat[i][j] = js[i * 4 + j].get<float>();
		}
	}
}

inline void to_json(nlohmann::json& js, const glm::mat4& mat)
{
	js = nlohmann::json::array();

	for (int32 i = 0; i < 4; ++i) {
		for (int32 j = 0; j < 4; ++j) {
			js.push_back(mat[i][j]);
		}
	}
}

} // namespace glm

namespace parsingaux {
inline std::string mat4_to_string(const glm::mat4& mat)
{
	std::string s;
	glm::mat4 m = glm::transpose(mat);
	for (int32 i = 0; i < 4; ++i) {
		s += '\n';
		for (int32 j = 0; j < 4; ++j) {
			s += std::to_string(m[i][j]) + ", ";
		}
	}
	s += '\n';
	return s;
}
} // namespace parsingaux

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

	if (v.size() > filter.size() && v.substr(v.size() - filter.size()) == filter) {
		v = v.substr(0, v.size() - filter.size());
	}
	return std::string{ v };
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
