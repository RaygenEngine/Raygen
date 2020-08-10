#pragma once

#include "ecs_universe/SceneComponentBase.h"

struct CLightBase : CSceneBase {
	glm::vec3 color{ 1.f };
	float intensity{ 30.f };

	bool hasShadow{ true };

	int32 shadowMapWidth{ 2048 };
	int32 shadowMapHeight{ 2048 };

	float near_{ 0.05f };
	float far_{ 20.0f };

	float maxShadowBias{ 0.005f };
	int32 samples{ 4 };
	float sampleInvSpread{ 1000.f };
};
