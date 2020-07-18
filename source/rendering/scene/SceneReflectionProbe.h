#pragma once
#include "rendering/scene/SceneStructs.h"

struct Ambient_Ubo {
	glm::vec4 color{};
};

struct SceneReflectionProbe : SceneStruct {
	SCENE_STRUCT(SceneReflectionProbe);

	Ambient_Ubo ubo;

	vl::GpuHandle<EnvironmentMap> envmap;

	void Build();
};
