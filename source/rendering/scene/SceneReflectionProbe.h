#pragma once
#include "rendering/scene/SceneStructs.h"
#include "rendering/objects/Cubemap.h"
#include "rendering/assets/GpuCubemap.h"

struct Ambient_Ubo {
	glm::vec4 color{};
};

struct SceneReflectionProbe : SceneStruct<Ambient_Ubo> {

	vl::GpuHandle<Cubemap> cubemap;

	SceneReflectionProbe();

	void Build();
};
