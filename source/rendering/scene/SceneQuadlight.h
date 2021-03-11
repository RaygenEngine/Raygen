#pragma once
#include "rendering/scene/SceneStructs.h"

struct Quadlight_Ubo {
	XMFLOAT3A center{};
	XMFLOAT3A normal{}; // TODO: pass quat
	XMFLOAT3A right{};
	XMFLOAT3A up{};

	XMFLOAT3A color{};

	float intensity{};
	float constantTerm{};
	float linearTerm{};
	float quadraticTerm{};

	float cosAperture{};

	float radius{};

	int32 samples{};
	int32 hasShadow{};
};

struct SceneQuadlight : SceneStruct {
	SCENE_STRUCT(SceneQuadlight);
	Quadlight_Ubo ubo;

	XMFLOAT4X4A transform;
};
