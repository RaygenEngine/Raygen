#pragma once
#include "rendering/scene/SceneStructs.h"

struct SceneGeometry {
	glm::mat4 transform;
	glm::mat4 prevTransform;

	vl::GpuHandle<Mesh> mesh;
};

struct SceneAnimatedGeometry {
	glm::mat4 transform{};
	vl::GpuHandle<SkinnedMesh> mesh;
	PodHandle<SkinnedMesh> meshPod;

	std::vector<glm::mat4> jointMatrices;
};
