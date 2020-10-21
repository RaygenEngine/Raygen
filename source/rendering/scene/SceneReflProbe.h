#pragma once
#include "rendering/scene/SceneStructs.h"

struct Reflprobe_UBO {
	glm::vec4 position{};
	float innerRadius;
	float outerRadius;
};

struct SceneReflprobe : SceneStruct {
	SCENE_STRUCT(SceneReflprobe);

	Reflprobe_UBO ubo;

	vl::GpuHandle<EnvironmentMap> envmap;


	void Build();
};
