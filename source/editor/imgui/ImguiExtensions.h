#pragma once

#include "glm.hpp"
#include "imgui/imgui_stdlib.h"

namespace ImUtil {
// TODO: implement on ImGui side.
static float* FromVec3(glm::vec3& vec3)
{
	return reinterpret_cast<float*>(&vec3);
}
static float* FromVec4(glm::vec4& vec4)
{
	return reinterpret_cast<float*>(&vec4);
}


} // namespace ImUtil