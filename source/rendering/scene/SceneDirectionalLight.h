#pragma once
#include "rendering/scene/SceneStructs.h"
#include "rendering/wrappers/RDepthmap.h"
#include "core/math-ext/Frustum.h"

struct DirectionalLight_Ubo {
	glm::vec4 forward{};

	// Lightmap
	glm::mat4 viewProj{};
	glm::vec4 color{};

	float intensity{};
};

struct SceneDirectionalLight : SceneStruct {
	SCENE_STRUCT(SceneDirectionalLight);

	DirectionalLight_Ubo ubo;

	UniquePtr<vl::RDepthmap> shadowmap;

	glm::vec3 up;

	void ResizeShadowmap(uint32 width, uint32 height);
	void UpdateBox(math::Frustum frustum, glm::vec3 apex);
};
