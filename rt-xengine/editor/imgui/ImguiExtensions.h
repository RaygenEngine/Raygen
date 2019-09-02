#pragma once

#include "glm.hpp"
#include "imgui/imgui_stdlib.h"

namespace ImUtil {
static float* FromVec3(glm::vec3& vec3)
{
	return reinterpret_cast<float*>(&vec3);
}

static bool AddReflector(Reflector& reflector) 
{
	bool dirty = false;
	for (auto& prop : reflector.GetProperties())
	{
		auto str = prop.GetName().c_str();

		dirty |= prop.SwitchOnType(
			[&str](int& ref) {
			return ImGui::DragInt(str, &ref, 0.1f);
		},
			[&str](bool& ref) {
			return ImGui::Checkbox(str, &ref);
		},
			[&str](float& ref) {
			return ImGui::DragFloat(str, &ref, 0.01f);
		},
			[&str](glm::vec3& ref) {
			return ImGui::DragFloat3(str, ImUtil::FromVec3(ref), 0.01f);
		},
			[&str](std::string& ref) {
			return ImGui::InputText(str, &ref);
		});
	}
	return dirty;
}


}