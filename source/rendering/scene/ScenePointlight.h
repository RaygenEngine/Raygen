#pragma once
#include "rendering/scene/SceneStructs.h"

struct Pointlight_Ubo {
	XMFLOAT3A position{};
	XMFLOAT3A color{};

	float intensity{};

	float constantTerm{};
	float linearTerm{};
	float quadraticTerm{};

	float radius{};

	int32 samples{};
	int32 hasShadow{};
};

struct ScenePointlight : SceneStruct {
	SCENE_STRUCT(ScenePointlight);
	Pointlight_Ubo ubo;

	XMFLOAT4X4 volumeTransform;
};
