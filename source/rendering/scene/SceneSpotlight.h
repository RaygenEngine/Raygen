#pragma once
#include "rendering/scene/SceneStructs.h"
#include "rendering/structures/Depthmap.h"

struct Spotlight_Ubo {
	glm::vec4 position{};
	glm::vec4 forward{};

	// Lightmap
	glm::mat4 viewProj{};
	glm::vec4 color{};

	float intensity{};

	float near_{};
	float far_{};

	float outerCutOff{};
	float innerCutOff{};

	float constantTerm{};
	float linearTerm{};
	float quadraticTerm{};

	float maxShadowBias{};
	int32 samples{};
	float sampleInvSpread{};
};

struct SceneSpotlight : SceneStruct {
	SCENE_STRUCT(SceneSpotlight);
	Spotlight_Ubo ubo;

	InFlightResources<vl::Depthmap> shadowmap;


	std::string name;
	void MaybeResizeShadowmap(uint32 width, uint32 height);
};
