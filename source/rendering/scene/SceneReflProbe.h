#pragma once
#include "rendering/scene/SceneStructs.h"

struct Ambient_Ubo {
	glm::vec4 color{};
};

struct SceneReflProbe : SceneStruct {
	SCENE_STRUCT(SceneReflProbe);

	Ambient_Ubo ubo;

	vl::GpuHandle<EnvironmentMap> envmap;
	glm::vec3 position;


	void Build();
};
