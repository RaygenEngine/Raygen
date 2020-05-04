#pragma once
#include "rendering/scene/SceneStructs.h"
#include "rendering/assets/GpuEnvironmentMap.h"

struct Ambient_Ubo {
	glm::vec4 color{};
};

struct SceneReflectionProbe : SceneStruct<Ambient_Ubo> {
	// WIP: add width, height (from world) -> Build prefiltered maps using width and height
	vl::GpuHandle<EnvironmentMap> envmap;

	void Build();
};
