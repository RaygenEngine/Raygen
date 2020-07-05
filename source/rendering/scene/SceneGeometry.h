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
	glm::mat4 jointMatrices[2];
};

struct SceneAnimatedGeometry : SceneStruct<AnimatedGeometry_Ubo> {
	glm::mat4 transform;
	PodHandle<SkinnedMesh> model;
	// PodHandle<Animation> animation;

	// std::vector<glm::mat4> jointMatrices;

	AnimatedGeometryNode* node;
	std::vector<bool> isDirty;
};
