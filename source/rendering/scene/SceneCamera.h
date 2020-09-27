#pragma once
#include "core/math-ext/Frustum.h"
#include "rendering/scene/SceneStructs.h"

struct Camera_Ubo {
	glm::vec4 position;
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 viewProj;
	glm::mat4 viewInv;
	glm::mat4 projInv;
	glm::mat4 viewProjInv;

	float time;
};

struct SceneCamera : SceneStruct {
	SCENE_STRUCT(SceneCamera);
	Camera_Ubo ubo;

	glm::mat4 prevViewProj{};
	math::Frustum frustum;
};
