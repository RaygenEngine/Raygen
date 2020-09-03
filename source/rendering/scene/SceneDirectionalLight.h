#pragma once
#include "rendering/scene/SceneStructs.h"
#include "rendering/structures/Depthmap.h"

namespace math {
// awful
struct Frustum;
} // namespace math

struct DirectionalLight_Ubo {
	glm::vec4 front{};

	// Lightmap
	glm::mat4 viewProj{};
	glm::vec4 color{};

	float intensity{};

	float maxShadowBias{};
	int32 samples{};
	float sampleInvSpread{};
};

struct SceneDirectionalLight : SceneStruct {
	SCENE_STRUCT(SceneDirectionalLight);

	DirectionalLight_Ubo ubo;

	InFlightResources<vl::Depthmap> shadowmap;

	glm::vec3 up;

	std::string name;

	void MaybeResizeShadowmap(uint32 width, uint32 height);
	void UpdateBox(const math::Frustum& frustum, glm::vec3 apex);
};
