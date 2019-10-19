#pragma once

#include "world/nodes/Node.h"
#include "asset/pods/ModelPod.h"

class GeometryNode : public Node {
	REFLECTED_NODE(GeometryNode, Node, DF_FLAGS(ModelChange)) { REFLECT_VAR(m_model).OnDirty(DF::ModelChange); }

	PodHandle<ModelPod> m_model;

public:
	[[nodiscard]] PodHandle<ModelPod> GetModel() const { return m_model; }

	void DirtyUpdate(DirtyFlagset dirtyFlags) override;
};
