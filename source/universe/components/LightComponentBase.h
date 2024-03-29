#pragma once

#include "universe/SceneComponentBase.h"


struct CLightBase : CSceneBase {

	enum LightType
	{
		Static,
		Dynamic,
	};

	glm::vec3 color{ 1.f };
	float intensity{ 30.f };

	bool hasShadow{ true };

	int32 shadowMapWidth{ 512 };
	int32 shadowMapHeight{ 512 };

	float _near{ 0.25f };
	float _far{ 10.0f };

	float maxShadowBias{ 0.005f };

	int32 samples{ 1 };
	float radius{ 0.002f };

	LightType type{ Dynamic };
};
