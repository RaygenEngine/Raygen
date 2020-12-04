#pragma once
#include "rendering/scene/SceneStructs.h"

namespace math {
// awful
struct Frustum;
} // namespace math


struct Dirlight_Ubo {
	glm::vec4 front{};

	// Lightmap
	glm::mat4 viewProj{};
	glm::vec4 color{};

	float intensity{};

	float maxShadowBias{};
	int32 samples{};
	float sampleInvSpread{};
	int32 hasShadow{};
};

struct SceneDirlight : SceneStruct {
	SCENE_STRUCT(SceneDirlight);

	Dirlight_Ubo ubo;

	InFlightResources<vl::RenderingPassInstance> shadowmapPass;
	InFlightResources<vk::DescriptorSet> shadowmapDescSet;

	vk::Sampler depthSampler;

	glm::vec3 up;

	std::string name;

	void MaybeResizeShadowmap(uint32 width, uint32 height);
	void UpdateBox(const math::Frustum& frustum, glm::vec3 apex);
};
