#pragma once
#include "rendering/scene/SceneStructs.h"

struct Pointlight_Ubo {
	glm::vec4 position{};
	glm::vec4 color{};

	float intensity{};

	float constantTerm{};
	float linearTerm{};
	float quadraticTerm{};

	int32 samples{};
	int32 hasShadow{};
};

struct ScenePointlight : SceneStruct {
	SCENE_STRUCT(ScenePointlight);
	Pointlight_Ubo ubo;

	glm::mat4 volumeTransform;
};
