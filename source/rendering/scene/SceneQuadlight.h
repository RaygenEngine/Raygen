#pragma once
#include "rendering/scene/SceneStructs.h"

struct Quadlight_Ubo {
	glm::vec4 position{};
	glm::vec4 front{};
	float scaleX{};
	float scaleY{};
	float pad0;
	float pad1;
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
