#pragma once
#include "rendering/scene/SceneStructs.h"

#if defined(near)
#	undef near
#endif

#if defined(far)
#	undef far
#endif

struct Spotlight_Ubo {
	XMFLOAT3A position{};
	XMFLOAT3A front{};

	// Lightmap
	XMFLOAT4X4A viewProj{};
	XMFLOAT3A color{};

	float intensity{};

	float near{};
	float far{};

	float outerCutOff{};
	float innerCutOff{};

	float constantTerm{};
	float linearTerm{};
	float quadraticTerm{};

	float maxShadowBias{};
	int32 samples{};
	float sampleInvSpread{};
	int32 hasShadow{};
};

struct SceneSpotlight : SceneStruct {
	SCENE_STRUCT(SceneSpotlight);
	Spotlight_Ubo ubo;

	std::string name;
};
