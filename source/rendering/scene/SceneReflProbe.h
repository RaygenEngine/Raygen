#pragma once
#include "rendering/scene/SceneStructs.h"

struct Reflprobe_UBO {
	glm::vec4 position{};
};

struct SceneReflprobe : SceneStruct {
	SCENE_STRUCT(SceneReflprobe);

	Reflprobe_UBO ubo;

	vl::GpuHandle<EnvironmentMap> envmap;
	glm::vec3 position;


	void Build();
};
