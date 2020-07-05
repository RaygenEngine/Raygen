#pragma once
#include "assets/pods/Mesh.h"
#include "rendering/assets/GpuAssetHandle.h"
#include "universe/nodes/geometry/GeometryNode.h"
#include "universe/nodes/geometry/AnimatedGeometryNode.h"
#include "rendering/scene/SceneStructs.h"

struct SceneGeometry {
	glm::mat4 transform;
	vl::GpuHandle<Mesh> model;

	GeometryNode* node;
	std::vector<bool> isDirty;
};

struct AnimatedGeometry_Ubo {
	std::array<glm::mat4, 2> jointMatrices;
};

struct SceneAnimatedGeometry : SceneStruct<AnimatedGeometry_Ubo> {
	glm::mat4 transform;
	vl::GpuHandle<SkinnedMesh> model;
	PodHandle<SkinnedMesh> modelPod;
	// PodHandle<Animation> animation;

	// std::vector<glm::mat4> jointMatrices;

	std::array<bool, 3> isDirty;
};
