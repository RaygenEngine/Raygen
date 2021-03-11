#pragma once
#include "rendering/scene/SceneStructs.h"

struct SceneGeometry {
	XMFLOAT4X4A transform;
	XMFLOAT4X4A prevTransform;

	GpuHandle<Mesh> mesh;
};

struct SceneAnimatedGeometry {
	XMFLOAT4X4A transform{};
	GpuHandle<SkinnedMesh> mesh;
	PodHandle<SkinnedMesh> meshPod;

	std::vector<glm::mat4> jointMatrices;
};
