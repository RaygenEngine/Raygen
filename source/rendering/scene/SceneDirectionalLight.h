#pragma once
#include "rendering/scene/SceneStructs.h"
#include "rendering/wrappers/RDepthmap.h"

namespace math {
// awful
struct Frustum;
} // namespace math

struct DirectionalLight_Ubo {
	glm::vec4 forward{};

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

	FrameArray<vl::RDepthmap> shadowmap;

	glm::vec3 up;

	std::string name;

	void MaybeResizeShadowmap(uint32 width, uint32 height);
	void UpdateBox(const math::Frustum& frustum, glm::vec3 apex);
};
