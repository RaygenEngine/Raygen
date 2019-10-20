#pragma once

#include "world/nodes/Node.h"
#include "asset/pods/ModelPod.h"
// WIP: move to cpp
#include "asset/AssetManager.h"

class GeometryNode : public Node {
	REFLECTED_NODE(GeometryNode, Node) { REFLECT_VAR(m_model); }

	PodHandle<ModelPod> m_model;

public:
	GeometryNode() { m_model = AssetManager::GetOrCreate<ModelPod>("/genEmptyModel"); }

	Box GetBBox() const override { return m_model.Lock()->bbox; }

	[[nodiscard]] PodHandle<ModelPod> GetModel() const { return m_model; }
};
