#pragma once

#include "world/nodes/Node.h"
#include "asset/loaders/GltfModelLoader.h"

class GeometryNode : public Node {
	REFLECTED_NODE(GeometryNode, Node) { REFLECT_VAR(m_model); }

	PodHandle<ModelPod> m_model;

public:
	[[nodiscard]] PodHandle<ModelPod> GetModel() const { return m_model; }
};
