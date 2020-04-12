#pragma once
#include "assets/pods/Mesh.h"
#include "rendering/assets/GpuAssetHandle.h"
#include "universe/nodes/geometry/GeometryNode.h"

struct SceneGeometry {
	glm::mat4 transform;
	vl::GpuHandle<Mesh> model;

	GeometryNode* node;
	std::vector<bool> isDirty;
};
