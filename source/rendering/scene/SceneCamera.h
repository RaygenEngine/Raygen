#pragma once
#include "rendering/scene/SceneStructs.h"

struct Camera_Ubo {
	glm::vec4 position;
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 viewProj;
	glm::mat4 viewInv;
	glm::mat4 projInv;
	glm::mat4 viewProjInv;
};

struct SceneCamera : SceneStruct<Camera_Ubo> {
};
