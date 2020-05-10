#pragma once
#include "rendering/scene/SceneStructs.h"
#include "rendering/objects/RDepthmap.h"

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
};

struct SceneSpotlight : SceneStruct<Spotlight_Ubo> {

	UniquePtr<vl::RDepthmap> shadowmap;

	void ResizeShadowmap(uint32 width, uint32 height);
};
