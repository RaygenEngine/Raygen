#pragma once
#include "rendering/scene/SceneStructs.h"

namespace math {
// awful
struct Frustum;
} // namespace math


struct Dirlight_Ubo {
	XMFLOAT3A front{};

	// Lightmap
	XMFLOAT4X4A viewProj{};
	XMFLOAT3A color{};

	float intensity{};

	float maxShadowBias{};
	int32 samples{};
	float sampleInvSpread{};
	int32 hasShadow{};
};

struct SceneDirlight : SceneStruct {
	SCENE_STRUCT(SceneDirlight);

	Dirlight_Ubo ubo;

	std::string name;
};
