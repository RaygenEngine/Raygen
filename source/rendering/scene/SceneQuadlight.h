#pragma once
#include "rendering/scene/SceneStructs.h"

struct Quadlight_Ubo {
	glm::vec4 center{};
	glm::vec4 normal{}; // WIP: pass quat
	glm::vec4 right{};
	glm::vec4 up{};

	glm::vec4 color{};

	float intensity{};
	float constantTerm{};
	float linearTerm{};
	float quadraticTerm{};

	float radius{};

	int32 samples{};
	int32 hasShadow{};
};

struct SceneQuadlight : SceneStruct {
	SCENE_STRUCT(SceneQuadlight);
	Quadlight_Ubo ubo;
};
