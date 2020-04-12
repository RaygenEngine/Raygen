#pragma once
#include "rendering/scene/SceneStructs.h"

struct Camera_Ubo {
	glm::vec4 position;
	glm::mat4 viewProj;
};

struct SceneCamera : SceneStruct<Camera_Ubo> {

	SceneCamera();
};
