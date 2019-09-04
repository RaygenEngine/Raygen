#pragma once

#include "glm.hpp"
#include "imgui/imgui_stdlib.h"

namespace ImUtil {
static float* FromVec3(glm::vec3& vec3)
{
	return reinterpret_cast<float*>(&vec3);
}


}