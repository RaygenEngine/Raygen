#pragma once

#include "world/nodes/Node.h"
#include "asset/loaders/GltfModelLoader.h"

class GeometryNode : public Node {
	REFLECTED_NODE(GeometryNode, Node) { REFLECT_VAR(m_model); }

	PodHandle<ModelPod> m_model;

public:
	GeometryNode(Node* parent)
		: Node(parent)
	{
	}

	Box GetBBox() const override { return m_model.Lock()->bbox; }

	[[nodiscard]] PodHandle<ModelPod> GetModel() const { return m_model; }
};
