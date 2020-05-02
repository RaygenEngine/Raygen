#pragma once
#include "rendering/scene/SceneStructs.h"
#include "rendering/objects/Cubemap.h"
#include "rendering/assets/GpuCubemap.h"

struct Ambient_Ubo {
	glm::vec4 color{};
};

struct SceneReflectionProbe : SceneStruct<Ambient_Ubo> {
	// WIP: add width, height (from world) -> Build prefiltered maps using width and height
	vl::GpuHandle<Cubemap> cubemap;

	uint32 irradianceMapResolution{ 512u };

	SceneReflectionProbe();

	void Build();
};
